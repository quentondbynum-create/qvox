#include "PluginEditor.h"
#include <BinaryData.h>

//==============================================================================
const juce::Colour WarmLookAndFeel::kAmber = juce::Colour (0xffF5A623);
const juce::Colour WarmLookAndFeel::kGold  = juce::Colour (0xffFFD27A);
const juce::Colour WarmLookAndFeel::kCream = juce::Colour (0xffFFF3E0);
const juce::Colour WarmLookAndFeel::kPanel = juce::Colour (0xff2E1C0C);

void WarmLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider&)
{
    const float cx = x + width * 0.5f;
    const float cy = y + height * 0.5f;
    const float radius = juce::jmin (width, height) * 0.5f - 14.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Soft outer glow.
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (kAmber.withAlpha (0.05f * i));
        g.fillEllipse (cx - radius - i * 7.0f, cy - radius - i * 7.0f,
                       (radius + i * 7.0f) * 2.0f, (radius + i * 7.0f) * 2.0f);
    }

    // Knob body.
    juce::ColourGradient bodyGrad (juce::Colour (0xff3A2410), cx, cy - radius,
                                   juce::Colour (0xff1E1006), cx, cy + radius, false);
    g.setGradientFill (bodyGrad);
    g.fillEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f);
    g.setColour (kAmber.withAlpha (0.35f));
    g.drawEllipse (cx - radius, cy - radius, radius * 2.0f, radius * 2.0f, 1.5f);

    // Track arc (dim).
    juce::Path trackArc;
    trackArc.addCentredArc (cx, cy, radius + 7.0f, radius + 7.0f,
                            0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff5A3A1A));
    g.strokePath (trackArc, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

    // Value arc (glowing amber).
    if (sliderPos > 0.001f)
    {
        juce::Path valueArc;
        valueArc.addCentredArc (cx, cy, radius + 7.0f, radius + 7.0f,
                                0.0f, rotaryStartAngle, angle, true);
        g.setColour (kAmber.withAlpha (0.25f));
        g.strokePath (valueArc, juce::PathStrokeType (9.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.setColour (kAmber);
        g.strokePath (valueArc, juce::PathStrokeType (5.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.setColour (kGold);
        g.strokePath (valueArc, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    // Needle.
    const float needleLen = radius * 0.72f;
    const float nx = cx + std::sin (angle) * needleLen;
    const float ny = cy - std::cos (angle) * needleLen;
    g.setColour (kCream);
    g.drawLine (cx, cy, nx, ny, 3.0f);
    g.setColour (kAmber);
    g.fillEllipse (cx - 4.0f, cy - 4.0f, 8.0f, 8.0f);
}

void WarmLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                        bool, bool)
{
    auto r = button.getLocalBounds().toFloat().reduced (1.0f);
    const bool on = button.getToggleState();

    if (on)
    {
        g.setColour (kAmber);
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0xff2A1708));
    }
    else
    {
        g.setColour (juce::Colour (0xff3A2410));
        g.fillRoundedRectangle (r, 6.0f);
        g.setColour (kAmber.withAlpha (0.7f));
        g.drawRoundedRectangle (r, 6.0f, 1.0f);
        g.setColour (kCream.withAlpha (0.85f));
    }

    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText (button.getButtonText(), r, juce::Justification::centred);
}

void WarmLookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto r = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);
    g.setColour (kPanel);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (kAmber.withAlpha (0.6f));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    g.setColour (kCream);
    g.setFont (13.0f);
    g.drawText (box.getText(), r.reduced (8.0f, 0.0f),
                juce::Justification::centredLeft, true);
}

void WarmLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (juce::Colour (0xff1E1006));
    g.setColour (kAmber.withAlpha (0.4f));
    g.drawRect (0, 0, width, height, 1);
}

void WarmLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool, bool isHighlighted,
                                         bool, bool, const juce::String& text,
                                         const juce::String&, const juce::Drawable*,
                                         const juce::Colour*)
{
    if (isSeparator)
    {
        g.setColour (kAmber.withAlpha (0.3f));
        g.fillRect (area.getX(), area.getCentreY(), area.getWidth(), 1);
        return;
    }

    if (isHighlighted)
    {
        g.setColour (kAmber.withAlpha (0.25f));
        g.fillRect (area);
    }

    const bool isHeading = text.startsWith ("—") || area.getHeight() < 22;
    g.setColour (isHeading ? kAmber : kCream);
    g.setFont (juce::Font (isHeading ? 11.0f : 13.0f,
                           isHeading ? juce::Font::bold : juce::Font::plain));
    g.drawText (text, area.reduced (10, 0), juce::Justification::centredLeft, true);
}

//==============================================================================
const std::vector<QVoxFactoryPreset> QVoxEditor::factoryPresets =
{
    { "Init",        0.0f,  0.0f,  0.0f,  0.0f },
    { "Radio Ready", 45.0f, 60.0f, 25.0f, 40.0f },
    { "Warm R&B",    35.0f, 70.0f, 35.0f, 25.0f },
    { "Crisp Pop",   60.0f, 55.0f, 20.0f, 65.0f },
    { "Lo-fi Vox",   30.0f, 80.0f, 55.0f, 10.0f },
};

//==============================================================================
QVoxEditor::QVoxEditor (QVoxProcessor& p)
    : AudioProcessorEditor (p), processor (p)
{
    setSize (720, 400);

    logoImage = juce::ImageCache::getFromMemory (BinaryData::headroomlogo_png,
                                                 BinaryData::headroomlogo_pngSize);

    auto setupKnob = [this] (juce::Slider& s, juce::Label& label, const juce::String& text)
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        s.setLookAndFeel (&warmLaf);
        addAndMakeVisible (s);

        label.setText (text, juce::dontSendNotification);
        label.setFont (juce::Font (17.0f, juce::Font::bold));
        label.setColour (juce::Label::textColourId, WarmLookAndFeel::kCream);
        label.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (label);
    };

    setupKnob (deessKnob, deessLabel, "DE-ESS");
    setupKnob (compKnob,  compLabel,  "COMP");
    setupKnob (dblKnob,   dblLabel,   "DOUBLE");
    setupKnob (airKnob,   airLabel,   "AIR");

    auto setupBypass = [this] (juce::ToggleButton& b)
    {
        b.setButtonText ("BYP");
        b.setClickingTogglesState (true);
        b.setLookAndFeel (&warmLaf);
        addAndMakeVisible (b);
    };

    setupBypass (deessBypassBtn);
    setupBypass (compBypassBtn);
    setupBypass (dblBypassBtn);
    setupBypass (airBypassBtn);

    deessAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, QVoxProcessor::kDeess, deessKnob);
    compAttach  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, QVoxProcessor::kComp, compKnob);
    dblAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, QVoxProcessor::kDbl, dblKnob);
    airAttach   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processor.apvts, QVoxProcessor::kAir, airKnob);

    deessBypAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, QVoxProcessor::kDeessBypass, deessBypassBtn);
    compBypAttach  = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, QVoxProcessor::kCompBypass, compBypassBtn);
    dblBypAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, QVoxProcessor::kDblBypass, dblBypassBtn);
    airBypAttach   = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        processor.apvts, QVoxProcessor::kAirBypass, airBypassBtn);

    // ---- preset menu ----
    presetBox.setLookAndFeel (&warmLaf);
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        for (auto& e : presetEntries)
            if (e.id == id) { applyPreset (e); break; }
    };
    addAndMakeVisible (presetBox);

    heartBtn.setLookAndFeel (&warmLaf);
    heartBtn.setColour (juce::TextButton::textColourOnId, WarmLookAndFeel::kAmber);
    heartBtn.setColour (juce::TextButton::textColourOffId, WarmLookAndFeel::kCream.withAlpha (0.5f));
    heartBtn.onClick = [this] { toggleHeart(); };
    addAndMakeVisible (heartBtn);

    for (auto* b : { &prevBtn, &nextBtn })
    {
        b->setLookAndFeel (&warmLaf);
        b->setColour (juce::TextButton::textColourOnId, WarmLookAndFeel::kCream);
        b->setColour (juce::TextButton::textColourOffId, WarmLookAndFeel::kCream);
        addAndMakeVisible (b);
    }
    prevBtn.onClick = [this] { cyclePreset (-1); };
    nextBtn.onClick = [this] { cyclePreset (+1); };

    saveBtn.setLookAndFeel (&warmLaf);
    saveBtn.setColour (juce::TextButton::textColourOnId, WarmLookAndFeel::kAmber);
    saveBtn.setColour (juce::TextButton::textColourOffId, WarmLookAndFeel::kCream);
    saveBtn.onClick = [this] { promptSavePreset(); };
    addAndMakeVisible (saveBtn);

    loadFavorites();
    rebuildPresetList();
}

QVoxEditor::~QVoxEditor()
{
    for (auto* s : { &deessKnob, &compKnob, &dblKnob, &airKnob })
        s->setLookAndFeel (nullptr);
    for (auto* b : { &deessBypassBtn, &compBypassBtn, &dblBypassBtn, &airBypassBtn })
        b->setLookAndFeel (nullptr);
    for (auto* b : { &heartBtn, &prevBtn, &nextBtn, &saveBtn })
        b->setLookAndFeel (nullptr);
    presetBox.setLookAndFeel (nullptr);
}

//==============================================================================
void QVoxEditor::paint (juce::Graphics& g)
{
    // Warm espresso gradient.
    juce::ColourGradient bg (juce::Colour (0xff2A1608), 0, 0,
                             juce::Colour (0xff140B04), 0, (float) getHeight(), false);
    g.setGradientFill (bg);
    g.fillAll();

    // Header divider.
    g.setColour (WarmLookAndFeel::kAmber.withAlpha (0.28f));
    g.fillRect (0, 68, getWidth(), 1);

    // Logo + bold title.
    if (logoImage.isValid())
        g.drawImage (logoImage, 16, 18, 32, 32, 0, 0,
                     logoImage.getWidth(), logoImage.getHeight());

    g.setColour (WarmLookAndFeel::kCream);
    g.setFont (juce::Font (30.0f, juce::Font::bold));
    g.drawText ("QVOX", 58, 10, 200, 34, juce::Justification::left);

    g.setColour (WarmLookAndFeel::kAmber);
    g.setFont (juce::Font (11.0f, juce::Font::bold));
    g.drawText ("V O C A L   C H A I N", 60, 42, 220, 16, juce::Justification::left);
}

void QVoxEditor::resized()
{
    // Header: preset controls right-aligned.
    presetBox.setBounds (330, 18, 210, 32);
    heartBtn.setBounds  (548, 18, 36, 32);
    prevBtn.setBounds   (590, 18, 30, 32);
    nextBtn.setBounds   (624, 18, 30, 32);
    saveBtn.setBounds   (292, 18, 32, 32);

    // Four hero knobs, evenly spaced.
    const int knobD = 140;
    const int centersX[4] = { 90, 270, 450, 630 };
    juce::Slider* knobs[4] = { &deessKnob, &compKnob, &dblKnob, &airKnob };
    juce::Label* labels[4] = { &deessLabel, &compLabel, &dblLabel, &airLabel };
    juce::ToggleButton* byps[4] = { &deessBypassBtn, &compBypassBtn, &dblBypassBtn, &airBypassBtn };

    for (int i = 0; i < 4; ++i)
    {
        knobs[i]->setBounds (centersX[i] - knobD / 2, 96, knobD, knobD);
        labels[i]->setBounds (centersX[i] - 80, 242, 160, 24);
        byps[i]->setBounds (centersX[i] - 38, 272, 76, 26);
    }
}

//==============================================================================
juce::File QVoxEditor::presetDir() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Headroom Audio/QVox/Presets");
}

juce::File QVoxEditor::favoritesFile() const
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
        .getChildFile ("Headroom Audio/QVox/favorites.txt");
}

void QVoxEditor::loadFavorites()
{
    favoriteNames.clear();
    auto f = favoritesFile();
    if (f.existsAsFile())
        f.readLines (favoriteNames);
    favoriteNames.removeEmptyStrings();
}

void QVoxEditor::saveFavorites()
{
    auto f = favoritesFile();
    f.getParentDirectory().createDirectory();
    f.replaceWithText (favoriteNames.joinIntoString ("\n"));
}

bool QVoxEditor::isFavorite (const juce::String& name) const
{
    return favoriteNames.contains (name, true);
}

static juce::String stripHeart (juce::String s)
{
    return s.replace ("♥ ", "").trim();
}

void QVoxEditor::rebuildPresetList()
{
    const int prevId = presetBox.getSelectedId();
    const juce::String prevName = stripHeart (presetBox.getText());

    presetBox.clear();
    presetEntries.clear();

    int nextId = 1;

    auto addEntry = [&] (const juce::String& displayName, const PresetEntry& e)
    {
        PresetEntry entry = e;
        entry.id = nextId;
        entry.name = displayName;
        presetEntries.push_back (entry);
        presetBox.addItem (displayName, nextId);
        ++nextId;
    };

    // User preset files.
    juce::Array<juce::File> userFiles;
    auto dir = presetDir();
    if (dir.isDirectory())
        dir.findChildFiles (userFiles, juce::File::findFiles, false, "*.qvoxpreset");

    auto findEntry = [&] (const juce::String& name, PresetEntry& out) -> bool
    {
        for (size_t i = 0; i < factoryPresets.size(); ++i)
            if (juce::String (factoryPresets[i].name) == name)
            {
                out = {};
                out.isFactory = true;
                out.factoryIndex = (int) i;
                return true;
            }
        for (auto& f : userFiles)
            if (f.getFileNameWithoutExtension() == name)
            {
                out = {};
                out.isUser = true;
                out.userFile = f;
                return true;
            }
        return false;
    };

    // Favorites section.
    juce::StringArray favsAdded;
    for (auto& fav : favoriteNames)
    {
        PresetEntry e;
        if (findEntry (fav, e))
        {
            if (favsAdded.isEmpty())
                presetBox.addSectionHeading ("— ♥ FAVORITES —");
            addEntry ("♥ " + fav, e);
            favsAdded.add (fav);
        }
    }

    // Factory section.
    presetBox.addSectionHeading ("— FACTORY —");
    for (size_t i = 0; i < factoryPresets.size(); ++i)
    {
        PresetEntry e;
        e.isFactory = true;
        e.factoryIndex = (int) i;
        addEntry (factoryPresets[i].name, e);
    }

    // User section.
    if (! userFiles.isEmpty())
    {
        presetBox.addSectionHeading ("— USER —");
        for (auto& f : userFiles)
        {
            PresetEntry e;
            e.isUser = true;
            e.userFile = f;
            addEntry (f.getFileNameWithoutExtension(), e);
        }
    }

    // Restore previous selection by name (IDs shift on rebuild).
    if (prevName.isNotEmpty())
        for (auto& e : presetEntries)
            if (stripHeart (e.name) == prevName)
            {
                presetBox.setSelectedId (e.id, juce::dontSendNotification);
                break;
            }
    juce::ignoreUnused (prevId);

    // Heart reflects current selection.
    const juce::String cur = stripHeart (presetBox.getText());
    heartBtn.setToggleState (isFavorite (cur), juce::dontSendNotification);
    heartBtn.setButtonText (isFavorite (cur) ? "♥" : "♡");
}

void QVoxEditor::setParam01 (const char* paramId, float value01)
{
    if (auto* p = processor.apvts.getParameter (paramId))
        p->setValueNotifyingHost (p->convertTo0to1 (juce::jlimit (0.0f, 1.0f, value01)));
}

void QVoxEditor::applyFactoryPreset (const QVoxFactoryPreset& p)
{
    setParam01 (QVoxProcessor::kDeess, p.deess / 100.0f);
    setParam01 (QVoxProcessor::kComp,  p.comp  / 100.0f);
    setParam01 (QVoxProcessor::kDbl,   p.dbl   / 100.0f);
    setParam01 (QVoxProcessor::kAir,   p.air   / 100.0f);
}

void QVoxEditor::applyUserPresetFile (const juce::File& f)
{
    if (auto xml = juce::XmlDocument::parse (f))
    {
        juce::ValueTree vt = juce::ValueTree::fromXml (*xml);
        if (! vt.isValid() || ! vt.hasType ("QVoxPreset"))
            return;
        setParam01 (QVoxProcessor::kDeess, (float) vt.getProperty ("deess", 0.0));
        setParam01 (QVoxProcessor::kComp,  (float) vt.getProperty ("comp",  0.0));
        setParam01 (QVoxProcessor::kDbl,   (float) vt.getProperty ("dbl",   0.0));
        setParam01 (QVoxProcessor::kAir,   (float) vt.getProperty ("air",   0.0));
    }
}

void QVoxEditor::applyPreset (const PresetEntry& e)
{
    if (e.isFactory && e.factoryIndex >= 0
        && e.factoryIndex < (int) factoryPresets.size())
        applyFactoryPreset (factoryPresets[(size_t) e.factoryIndex]);
    else if (e.isUser)
        applyUserPresetFile (e.userFile);

    const juce::String cur = stripHeart (presetBox.getText());
    heartBtn.setToggleState (isFavorite (cur), juce::dontSendNotification);
    heartBtn.setButtonText (isFavorite (cur) ? "♥" : "♡");
}

void QVoxEditor::toggleHeart()
{
    const juce::String cur = stripHeart (presetBox.getText());
    if (cur.isEmpty())
        return;

    if (isFavorite (cur))
        favoriteNames.removeString (cur, true);
    else
        favoriteNames.add (cur);

    saveFavorites();
    const int keepId = presetBox.getSelectedId();
    rebuildPresetList();
    // Re-select the same preset if still present.
    for (auto& e : presetEntries)
        if (stripHeart (e.name) == cur)
        {
            presetBox.setSelectedId (e.id, juce::dontSendNotification);
            break;
        }
    juce::ignoreUnused (keepId);

    heartBtn.setToggleState (isFavorite (cur), juce::dontSendNotification);
    heartBtn.setButtonText (isFavorite (cur) ? "♥" : "♡");
}

void QVoxEditor::cyclePreset (int direction)
{
    if (presetEntries.empty())
        return;

    int idx = 0;
    const int curId = presetBox.getSelectedId();
    for (size_t i = 0; i < presetEntries.size(); ++i)
        if (presetEntries[i].id == curId)
        {
            idx = (int) i;
            break;
        }

    idx = (idx + direction + (int) presetEntries.size()) % (int) presetEntries.size();
    presetBox.setSelectedId (presetEntries[(size_t) idx].id, juce::sendNotification);
}

void QVoxEditor::promptSavePreset()
{
    auto* aw = new juce::AlertWindow ("Save Preset", "Name your vocal chain preset:",
                                      juce::AlertWindow::NoIcon);
    aw->addTextEditor ("name", "", "Preset name");
    aw->addButton ("Save", 1, juce::KeyPress (juce::KeyPress::returnKey));
    aw->addButton ("Cancel", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    aw->enterModalState (true, juce::ModalCallbackFunction::create (
        [this, aw] (int result)
        {
            if (result == 1)
            {
                const juce::String name = aw->getTextEditorContents ("name").trim();
                if (name.isNotEmpty())
                {
                    juce::ValueTree vt ("QVoxPreset");
                    auto get01 = [this] (const char* pid)
                    {
                        if (auto* p = processor.apvts.getParameter (pid))
                            return p->convertTo0to1 (p->getValue());
                        return 0.0f;
                    };
                    vt.setProperty ("deess", get01 (QVoxProcessor::kDeess), nullptr);
                    vt.setProperty ("comp",  get01 (QVoxProcessor::kComp),  nullptr);
                    vt.setProperty ("dbl",   get01 (QVoxProcessor::kDbl),   nullptr);
                    vt.setProperty ("air",   get01 (QVoxProcessor::kAir),   nullptr);

                    auto dir = presetDir();
                    dir.createDirectory();
                    if (auto xml = vt.createXml())
                        xml->writeTo (dir.getChildFile (name + ".qvoxpreset"));

                    rebuildPresetList();
                    for (auto& e : presetEntries)
                        if (stripHeart (e.name) == name)
                        {
                            presetBox.setSelectedId (e.id, juce::dontSendNotification);
                            break;
                        }
                }
            }
            delete aw;
        }));
}
