/*
 * Copyright (c) 2024-2025 Igor Zinken https://www.igorski.nl
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

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor final : public juce::AudioProcessorEditor
{
    const int WIDTH  = 1441;
    const int HEIGHT = 830;
    
    const int VERSION_WIDTH  = 68;
    const int VERSION_HEIGHT = 25;
    const int SLIDER_WIDTH   = 80;
    const int ROTARY_SIZE    = 68;
    const int ROTARY_MARGIN  = 80;
    
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
    
        static const unsigned int BACKGROUND_COLOR = 0xff444444;
        static const unsigned int HIGHLIGHT_COLOR  = 0xffD92666;

        /* automatable parameters */

        juce::ToggleButton distActiveControl;
        juce::Slider mixControl;
        
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> distActiveAtt;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAtt;
      
        inline std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> createControl( const juce::String& title, juce::Slider& controlElement, bool rotary )
        {
            addAndMakeVisible( controlElement );

            controlElement.setTextBoxStyle( juce::Slider::NoTextBox, true, 0, 0 );
            controlElement.getLookAndFeel().setColour( juce::Slider::thumbColourId, juce::Colour( 0xffb6b6b6 ));
            
            if ( rotary ) {
                controlElement.setSliderStyle( juce::Slider::Rotary );
                controlElement.getLookAndFeel().setColour( juce::Slider::rotarySliderOutlineColourId, juce::Colour( BACKGROUND_COLOR ));
                controlElement.getLookAndFeel().setColour( juce::Slider::rotarySliderFillColourId,    juce::Colour( HIGHLIGHT_COLOR ));
            } else {
                controlElement.getLookAndFeel().setColour( juce::Slider::backgroundColourId, juce::Colour( BACKGROUND_COLOR ));
                controlElement.getLookAndFeel().setColour( juce::Slider::trackColourId, juce::Colour( HIGHLIGHT_COLOR ));
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
