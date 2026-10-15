#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
QVoxProcessor::QVoxProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "QVoxParams", createParameterLayout())
{
}

QVoxProcessor::~QVoxProcessor() = default;

juce::AudioProcessorValueTreeState::ParameterLayout QVoxProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    auto pct = [] (const char* id, const char* name)
    {
        return std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID (id, 1), name,
            juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f,
            juce::AudioParameterFloatAttributes().withLabel ("%"));
    };
    auto bypass = [] (const char* id, const char* name)
    {
        return std::make_unique<juce::AudioParameterBool> (juce::ParameterID (id, 1), name, false);
    };

    params.push_back (pct (kDeess, "De-ess"));
    params.push_back (bypass (kDeessBypass, "De-ess Bypass"));
    params.push_back (pct (kComp, "Compress"));
    params.push_back (bypass (kCompBypass, "Comp Bypass"));
    params.push_back (pct (kDbl, "Double"));
    params.push_back (bypass (kDblBypass, "Double Bypass"));
    params.push_back (pct (kAir, "Air"));
    params.push_back (bypass (kAirBypass, "Air Bypass"));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String QVoxProcessor::getName() const { return "QVox"; }
bool QVoxProcessor::acceptsMidi() const  { return false; }
bool QVoxProcessor::producesMidi() const { return false; }
bool QVoxProcessor::isMidiEffect() const { return false; }
double QVoxProcessor::getTailLengthSeconds() const { return 0.1; }

int QVoxProcessor::getNumPrograms() { return 1; }
int QVoxProcessor::getCurrentProgram() { return 0; }
void QVoxProcessor::setCurrentProgram (int) {}
const juce::String QVoxProcessor::getProgramName (int) { return {}; }
void QVoxProcessor::changeProgramName (int, const juce::String&) {}

bool QVoxProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

//==============================================================================
void QVoxProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sampleRateHz = sampleRate;
    juce::ignoreUnused (samplesPerBlock);

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };

    for (auto& d : deesser)
    {
        d.lpFilter.prepare (spec);
        *d.lpFilter.coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeLowPass (sampleRate, 5000.0f);
        d.env = 0.0f;
    }
    lastDeessXover = 5000.0f;

    for (auto& c : compCh)
        c.env = 0.0f;

    const int delayBufSize = (int) (0.064 * sampleRate) + 8;
    for (auto& d : doubler)
    {
        d.buf.assign ((size_t) delayBufSize, 0.0f);
        d.writePos = 0;
        d.lfoPhase = 0.0;
    }

    for (auto& f : airShelf)
    {
        f.prepare (spec);
        *f.coefficients =
            *juce::dsp::IIR::Coefficients<float>::makeHighShelf (sampleRate, 12000.0f, 0.7f, 0.0f);
    }
    lastAirGainDb = 0.0f;
}

void QVoxProcessor::releaseResources() {}

//==============================================================================
void QVoxProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numCh = juce::jmin (2, buffer.getNumChannels());
    const int n     = buffer.getNumSamples();
    const float sr  = (float) sampleRateHz;

    const float deessAmt = juce::jlimit (0.0f, 1.0f,
        apvts.getRawParameterValue (kDeess)->load() / 100.0f);
    const float compAmt  = juce::jlimit (0.0f, 1.0f,
        apvts.getRawParameterValue (kComp)->load() / 100.0f);
    const float dblAmt   = juce::jlimit (0.0f, 1.0f,
        apvts.getRawParameterValue (kDbl)->load() / 100.0f);
    const float airAmt   = juce::jlimit (0.0f, 1.0f,
        apvts.getRawParameterValue (kAir)->load() / 100.0f);

    const bool deessByp = apvts.getRawParameterValue (kDeessBypass)->load() > 0.5f;
    const bool compByp  = apvts.getRawParameterValue (kCompBypass)->load()  > 0.5f;
    const bool dblByp   = apvts.getRawParameterValue (kDblBypass)->load()   > 0.5f;
    const bool airByp   = apvts.getRawParameterValue (kAirBypass)->load()   > 0.5f;

    // ---- De-esser detector ballistics ----
    const float dAtk = std::exp (-1.0f / (0.0005f * sr)); // 0.5 ms
    const float dRel = std::exp (-1.0f / (0.050f  * sr)); // 50 ms
    const float dThreshDb = -8.0f - 24.0f * deessAmt;     // -8 .. -32 dB
    const float dMaxRedDb = 12.0f * deessAmt;

    // ---- Optical compressor ballistics ----
    const float cAtk = std::exp (-1.0f / (0.012f * sr));  // 12 ms
    const float cRel = std::exp (-1.0f / (0.180f * sr));  // 180 ms
    const float cThreshDb = -8.0f - 28.0f * compAmt;      // -8 .. -36 dB

    // ---- Air shelf: rebuild only on change ----
    const float airGainDb = 10.0f * airAmt;
    if (std::fabs (airGainDb - lastAirGainDb) > 0.05f)
    {
        for (auto& f : airShelf)
            *f.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (
                sampleRateHz, 12000.0f, 0.7f, airGainDb);
        lastAirGainDb = airGainDb;
    }

    const double lfoInc = 2.0 * juce::MathConstants<double>::pi * 0.4 / sampleRateHz;

    for (int ch = 0; ch < numCh; ++ch)
    {
        float* x = buffer.getWritePointer (ch);
        auto& ds = deesser[(size_t) ch];
        auto& cc = compCh[(size_t) ch];
        auto& db = doubler[(size_t) ch];
        auto& as = airShelf[(size_t) ch];
        const int dBufSize = (int) db.buf.size();

        for (int i = 0; i < n; ++i)
        {
            float s = x[i];

            // ---- 1. De-esser (split-band dynamic HF) ----
            if (! deessByp && deessAmt > 0.001f)
            {
                const float low  = ds.lpFilter.processSample (s);
                float high = s - low;

                const float ax = std::fabs (high);
                const float c = (ax > ds.env) ? dAtk : dRel;
                ds.env = c * ds.env + (1.0f - c) * ax;

                const float envDb = 20.0f * std::log10 (ds.env + 1.0e-9f);
                const float over = envDb - dThreshDb;
                float redDb = 0.0f;
                if (over > 0.0f)
                {
                    constexpr float knee = 6.0f;
                    constexpr float slope = 1.0f - 1.0f / 4.0f; // 4:1
                    redDb = (over < knee) ? -slope * over * over / (2.0f * knee)
                                          : -slope * (over - knee * 0.5f);
                    redDb = juce::jmax (redDb, -dMaxRedDb);
                }
                high *= dbToLin (redDb);
                s = low + high;
            }

            // ---- 2. Optical-style compressor ----
            if (! compByp && compAmt > 0.001f)
            {
                const float ax = std::fabs (s);
                const float c = (ax > cc.env) ? cAtk : cRel;
                cc.env = c * cc.env + (1.0f - c) * ax;

                const float envDb = 20.0f * std::log10 (cc.env + 1.0e-9f);
                const float over = envDb - cThreshDb;
                float redDb = 0.0f;
                if (over > 0.0f)
                {
                    constexpr float knee = 8.0f;
                    constexpr float slope = 1.0f - 1.0f / 3.0f; // 3:1
                    redDb = (over < knee) ? -slope * over * over / (2.0f * knee)
                                          : -slope * (over - knee * 0.5f);
                }
                s *= dbToLin (redDb);
            }

            // ---- 3. Doubler (modulated short delay, L/R antiphase) ----
            if (! dblByp && dblAmt > 0.001f)
            {
                db.buf[(size_t) db.writePos] = s;

                db.lfoPhase += lfoInc;
                if (db.lfoPhase > 2.0 * juce::MathConstants<double>::pi)
                    db.lfoPhase -= 2.0 * juce::MathConstants<double>::pi;

                const double phaseOff = (ch == 0) ? 0.0 : juce::MathConstants<double>::pi;
                const float delayMs = 20.0f + 4.0f * (float) std::sin (db.lfoPhase + phaseOff);
                float readPos = (float) db.writePos - delayMs * sr / 1000.0f;
                while (readPos < 0.0f) readPos += (float) dBufSize;

                const int i0 = (int) readPos % dBufSize;
                const int i1 = (i0 + 1) % dBufSize;
                const float frac = readPos - std::floor (readPos);
                const float wet = db.buf[(size_t) i0] * (1.0f - frac)
                                + db.buf[(size_t) i1] * frac;

                db.writePos = (db.writePos + 1) % dBufSize;

                s += wet * (dblAmt * 0.35f);
            }

            // ---- 4. Air (parallel saturated high-shelf) ----
            if (! airByp && airAmt > 0.001f)
            {
                const float hs = as.processSample (s);
                const float sat = std::tanh (hs * 1.5f) * 0.6f;
                s += sat * airAmt * 0.5f;
            }

            x[i] = s;
        }
    }

    // Clear any extra output channels.
    for (int ch = numCh; ch < buffer.getNumChannels(); ++ch)
        buffer.clear (ch, 0, n);
}

//==============================================================================
void QVoxProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void QVoxProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new QVoxProcessor();
}

juce::AudioProcessorEditor* QVoxProcessor::createEditor() { return new QVoxEditor (*this); }
bool QVoxProcessor::hasEditor() const { return true; }
