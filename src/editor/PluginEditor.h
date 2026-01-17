/*
 * Copyright (c) 2024-2026 Igor Zinken https://www.igorski.nl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "ThinRotaryLookAndFeel.h"
#include "Styles.h"
#include "../PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::AudioProcessorValueTreeState::Listener
{
    public:
        explicit AudioPluginAudioProcessorEditor( AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& state );
        ~AudioPluginAudioProcessorEditor() override;

        void paint( juce::Graphics& ) override;
        void resized() override;

    private:
        juce::AudioProcessorValueTreeState& parameters;

        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sliderAttachments;
        std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAttachments;

        int scaledWidth;
        int scaledHeight;

        AudioPluginAudioProcessor& audioProcessor;

        /* parameter listeners */

        void parameterChanged( const juce::String& parameterID, float newValue ) override;
        void setControlEnabled( juce::Slider& control, bool enabled );

        /* controllable parameters */

        juce::ToggleButton linkEnabledControl;
        juce::Slider splitFreqControl;
        juce::Slider splitModeControl;
        juce::Slider dryWetControl;
        juce::Slider loDistTypeControl;
        juce::Slider loDistInputControl;
        juce::Slider loDistDriveControl;
        juce::Slider loDistParamControl;
        juce::Slider hiDistTypeControl;
        juce::Slider hiDistInputControl;
        juce::Slider hiDistDriveControl;
        juce::Slider hiDistParamControl;
        
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> linkEnabledAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> splitFreqAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> splitModeAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dryWetAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loDistTypeAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loDistInputAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loDistDriveAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loDistParamAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiDistTypeAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiDistInputAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiDistDriveAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiDistParamAtt;

        // styles

        ThinRotaryLookAndFeel largeRotaryLNF;
        ThinRotaryLookAndFeel smallRotaryLNF;
        
        inline std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> createControl( const juce::String& title, juce::Slider& controlElement, bool rotary )
        {
            addAndMakeVisible( controlElement );

            controlElement.setTextBoxStyle( juce::Slider::NoTextBox, true, 0, 0 );
            controlElement.getLookAndFeel().setColour( juce::Slider::thumbColourId, juce::Colour( 0xffb6b6b6 ));
            
            if ( rotary ) {
                controlElement.setSliderStyle( juce::Slider::Rotary );
                controlElement.getLookAndFeel().setColour( juce::Slider::rotarySliderOutlineColourId, juce::Colour( Styles::BACKGROUND_COLOR ));
                controlElement.getLookAndFeel().setColour( juce::Slider::rotarySliderFillColourId, juce::Colour( Styles::HIGHLIGHT_COLOR ));
            } else {
                controlElement.getLookAndFeel().setColour( juce::Slider::backgroundColourId, juce::Colour( Styles::BACKGROUND_COLOR ));
                controlElement.getLookAndFeel().setColour( juce::Slider::trackColourId, juce::Colour( Styles::HIGHLIGHT_COLOR ));
            }
            controlElement.getLookAndFeel().setColour( juce::Slider::thumbColourId, juce::Colour( 0x00000000 ));

            return std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>( parameters, title, controlElement );
        }

        inline std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> createControl( const juce::String& title, juce::ToggleButton& controlElement )
        {
            addAndMakeVisible( controlElement );
            controlElement.setTitle( title );

            return std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>( parameters, title, controlElement );
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioPluginAudioProcessorEditor )
};
