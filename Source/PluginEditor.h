#pragma once

#include "PluginProcessor.h"

//==============================================================================
// Warm amber/gold look: dark knob body, glowing amber arc, bold labels.
class WarmLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override;

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted,
                           bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics& g, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override;

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted,
                            bool isTicked, bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override;

    void drawPopupMenuBackground (juce::Graphics& g, int width, int height) override;

    static const juce::Colour kAmber;
    static const juce::Colour kGold;
    static const juce::Colour kCream;
    static const juce::Colour kPanel;
};

//==============================================================================
struct QVoxFactoryPreset
{
    const char* name;
    float deess, comp, dbl, air;
};

//==============================================================================
class QVoxEditor : public juce::AudioProcessorEditor
{
public:
    explicit QVoxEditor (QVoxProcessor&);
    ~QVoxEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    QVoxProcessor& processor;
    WarmLookAndFeel warmLaf;

    juce::Image logoImage;

    // ---- hero knobs ----
    juce::Slider deessKnob, compKnob, dblKnob, airKnob;
    juce::Label  deessLabel, compLabel, dblLabel, airLabel;
    juce::ToggleButton deessBypassBtn, compBypassBtn, dblBypassBtn, airBypassBtn;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> deessAttach, compAttach,
        dblAttach, airAttach;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> deessBypAttach,
        compBypAttach, dblBypAttach, airBypAttach;

    // ---- preset menu ----
    juce::ComboBox presetBox;
    juce::TextButton heartBtn { "♥" };
    juce::TextButton prevBtn  { "‹" };
    juce::TextButton nextBtn  { "›" };
    juce::TextButton saveBtn  { "SAVE" };

    struct PresetEntry
    {
        int id = 0;
        juce::String name;
        bool isFactory = false;
        bool isUser = false;
        int factoryIndex = -1;
        juce::File userFile;
    };
    std::vector<PresetEntry> presetEntries;
    juce::StringArray favoriteNames;

    static const std::vector<QVoxFactoryPreset> factoryPresets;

    juce::File presetDir() const;
    juce::File favoritesFile() const;
    void loadFavorites();
    void saveFavorites();
    bool isFavorite (const juce::String& name) const;
    void rebuildPresetList();
    void applyPreset (const PresetEntry& e);
    void applyFactoryPreset (const QVoxFactoryPreset& p);
    void applyUserPresetFile (const juce::File& f);
    void setParam01 (const char* paramId, float value01);
    void toggleHeart();
    void cyclePreset (int direction);
    void promptSavePreset();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QVoxEditor)
};
