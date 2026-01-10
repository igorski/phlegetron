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
    : AudioProcessorEditor( &p ), parameters( state )
{
    mixAtt = createControl( Parameters::DRY_WET_MIX, mixControl, true );

    scaledWidth  = static_cast<int>( ceil( WIDTH / 2 ));
    scaledHeight = static_cast<int>( ceil( HEIGHT / 2 ));

    setSize( scaledWidth, scaledHeight );
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    // nowt...
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
    
}