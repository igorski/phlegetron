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
#include "Parameters.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
// #include "BinaryData.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor( AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& state )
    : AudioProcessorEditor( &p ), parameters( state ), audioProcessor( p )
{
    scaledWidth  = static_cast<int>( ceil( WIDTH / 2 ));
    scaledHeight = static_cast<int>( ceil( HEIGHT / 2 ));

    setSize( scaledWidth, scaledHeight );

    // create UI controls for all plugin parameters

    splitEnabledAtt = createControl( Parameters::SPLIT_ENABLED, splitEnabledControl );
    splitFreqAtt    = createControl( Parameters::SPLIT_FREQ, splitFreqControl, true );

    // add listeners

    audioProcessor.parameters.addParameterListener( Parameters::SPLIT_ENABLED, this );
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    audioProcessor.parameters.removeParameterListener( Parameters::SPLIT_ENABLED, this );   
}

void AudioPluginAudioProcessorEditor::parameterChanged( const juce::String& id, float value )
{
    if ( id == Parameters::SPLIT_ENABLED ) {
        const bool enabled = ParameterUtilities::floatToBool( value );

        juce::MessageManager::callAsync([ this, enabled ] {
            handleSplitState( enabled );
        });
    }
}

void AudioPluginAudioProcessorEditor::handleSplitState( bool splitEnabled )
{
    // @todo this is test code, implement final design and business logic
    splitFreqControl.setEnabled( splitEnabled );
    // Optional: grey out visually
    splitFreqControl.setAlpha( splitEnabled ? 1.0f : 0.4f );
}

void AudioPluginAudioProcessorEditor::paint( juce::Graphics& g )
{
    g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ));

    // juce::Image background = juce::ImageCache::getFromMemory( BinaryData::background_png, BinaryData::background_pngSize );
    // g.drawImage( background, 0, 0, scaledWidth, scaledHeight, 0, 0, WIDTH, HEIGHT, false );
    /*
    int scaledVersionWidth  = static_cast<int>( ceil( VERSION_WIDTH  / 2 ));
    int scaledVersionHeight = static_cast<int>( ceil( VERSION_HEIGHT / 2 ));

    juce::Image version = juce::ImageCache::getFromMemory( BinaryData::version_png, BinaryData::version_pngSize );
    g.drawImage(
        version,
        scaledWidth - ( scaledVersionWidth + 17 ), scaledHeight - 37, scaledVersionWidth, scaledVersionHeight,
        0, 0, VERSION_WIDTH, VERSION_HEIGHT, false
    );
    */
}

void AudioPluginAudioProcessorEditor::resized()
{
    // @todo implement design
    int lowSectionX = 50;
    int lowSectionY = 50;

    splitFreqControl.setBounds ( lowSectionX, lowSectionY, ROTARY_SIZE, ROTARY_SIZE );
    splitEnabledControl.setBounds( lowSectionX + ROTARY_MARGIN, lowSectionY, ROTARY_SIZE, ROTARY_SIZE );
}