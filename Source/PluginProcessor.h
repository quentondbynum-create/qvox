#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <cmath>
#include <vector>

//==============================================================================
/*
    QVox — complete vocal chain in one plugin (Headroom Audio).

    Serial DSP chain:
      1. De-esser   — split-band dynamic HF compressor (5 kHz crossover,
                      fast compressor on the sibilance band)
      2. Compressor — smooth optical-style feedforward compressor
      3. Doubler    — short modulated delay, L/R antiphase LFO (width)
      4. Air        — parallel high-shelf exciter (12 kHz) + tanh saturation

    One "amount" knob per module (0-100%) plus a bypass per module.
*/
class QVoxProcessor : public juce::AudioProcessor
{
public:
    QVoxProcessor();
    ~QVoxProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // ---- parameters ----
    juce::AudioProcessorValueTreeState apvts;

    static constexpr const char* kDeess       = "deess";       // %, 0..100
    static constexpr const char* kDeessBypass = "deessBypass"; // bool
    static constexpr const char* kComp        = "comp";        // %, 0..100
    static constexpr const char* kCompBypass  = "compBypass";  // bool
    static constexpr const char* kDbl         = "dbl";         // %, 0..100
    static constexpr const char* kDblBypass   = "dblBypass";   // bool
    static constexpr const char* kAir         = "air";         // %, 0..100
    static constexpr const char* kAirBypass   = "airBypass";   // bool

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // ---- De-esser: split-band dynamic HF compressor ----
    struct DeesserChannel
    {
        juce::dsp::IIR::Filter<float> lpFilter; // 5 kHz lowpass; high band = in - low
        float env = 0.0f;
    };
    std::array<DeesserChannel, 2> deesser;

    // ---- Optical-style compressor ----
    struct CompChannel
    {
        float env = 0.0f;
    };
    std::array<CompChannel, 2> compCh;

    // ---- Doubler: modulated short delay ----
    struct DoublerChannel
    {
        std::vector<float> buf;
        int writePos = 0;
        double lfoPhase = 0.0;
    };
    std::array<DoublerChannel, 2> doubler;

    // ---- Air: high-shelf exciter ----
    std::array<juce::dsp::IIR::Filter<float>, 2> airShelf;

    double sampleRateHz = 44100.0;

    // Cached coefficient state so we only rebuild filters on change.
    float lastDeessXover = -1.0f;
    float lastAirGainDb  = -1000.0f;

    static float dbToLin (float db) noexcept { return std::pow (10.0f, db / 20.0f); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QVoxProcessor)
};
