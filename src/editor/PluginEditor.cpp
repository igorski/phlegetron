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
#include "PluginEditor.h"
#include "BinaryData.h"
#include "../Parameters.h"
#include "../PluginProcessor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor( AudioPluginAudioProcessor& p, juce::AudioProcessorValueTreeState& state )
    : AudioProcessorEditor( &p ), parameters( state ), audioProcessor( p )
{
    scaledWidth  = static_cast<int>( ceil( Styles::WIDTH / 2 ));
    scaledHeight = static_cast<int>( ceil( Styles::HEIGHT / 2 ));

    setSize( scaledWidth, scaledHeight );

    // create UI controls for all plugin parameters

    linkEnabledAtt = createControl( Parameters::LINK_ENABLED, linkEnabledControl );
    splitFreqAtt   = createControl( Parameters::SPLIT_FREQ, splitFreqControl, true );
    splitModeAtt   = createControl( Parameters::SPLIT_MODE, splitModeControl, false );
    dryWetAtt      = createControl( Parameters::DRY_WET_MIX, dryWetControl, false );
    
    loDistTypeAtt  = createControl( Parameters::LO_DIST_TYPE, loDistTypeControl, true );
    loDistInputAtt = createControl( Parameters::LO_DIST_INPUT, loDistInputControl, false );
    loDistDriveAtt = createControl( Parameters::LO_DIST_DRIVE, loDistDriveControl, false );
    loDistParamAtt = createControl( Parameters::LO_DIST_PARAM, loDistParamControl, false );
    
    hiDistTypeAtt  = createControl( Parameters::HI_DIST_TYPE, hiDistTypeControl, true );
    hiDistInputAtt = createControl( Parameters::HI_DIST_INPUT, hiDistInputControl, false );
    hiDistDriveAtt = createControl( Parameters::HI_DIST_DRIVE, hiDistDriveControl, false );
    hiDistParamAtt = createControl( Parameters::HI_DIST_PARAM, hiDistParamControl, false );

    largeRotaryLNF.setStrokeSize( 4.5f );
    smallRotaryLNF.setStrokeSize( 4.0f );

    splitFreqControl.setLookAndFeel( &largeRotaryLNF );
    loDistTypeControl.setLookAndFeel( &smallRotaryLNF );
    hiDistTypeControl.setLookAndFeel( &smallRotaryLNF );

    // add listeners

    audioProcessor.parameters.addParameterListener( Parameters::LINK_ENABLED, this );
    audioProcessor.parameters.addParameterListener( Parameters::LO_DIST_TYPE, this );
    audioProcessor.parameters.addParameterListener( Parameters::HI_DIST_TYPE, this );
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    audioProcessor.parameters.removeParameterListener( Parameters::LINK_ENABLED, this );
    audioProcessor.parameters.removeParameterListener( Parameters::LO_DIST_TYPE, this );
    audioProcessor.parameters.removeParameterListener( Parameters::HI_DIST_TYPE, this );
}

void AudioPluginAudioProcessorEditor::parameterChanged( const juce::String& id, float value )
{
    if ( id == Parameters::LINK_ENABLED ) {
        const bool enabled = ParameterUtilities::floatToBool( value );

        juce::MessageManager::callAsync([ this, enabled ] {
            setControlEnabled( hiDistTypeControl,  !enabled );
            setControlEnabled( hiDistInputControl, !enabled );
            setControlEnabled( hiDistDriveControl, !enabled );
            setControlEnabled( hiDistParamControl, !enabled );
        });
    }
    // @todo (?) more accurate enabled detection on <DistortionType> enum values!!
    // though technically this works well enough since enum increments in steps of 1 (and off == 0)

    if ( id == Parameters::LO_DIST_TYPE ) {
        const bool enabled = ParameterUtilities::floatToBool( value );

        juce::MessageManager::callAsync([ this, enabled ] {
            setControlEnabled( loDistInputControl, enabled ); 
            setControlEnabled( loDistDriveControl, enabled ); 
            setControlEnabled( loDistParamControl, enabled ); 
        });
    }
    if ( id == Parameters::HI_DIST_TYPE ) {
        const bool enabled = ParameterUtilities::floatToBool( value );

        juce::MessageManager::callAsync([ this, enabled ] {
            setControlEnabled( hiDistInputControl, enabled ); 
            setControlEnabled( hiDistDriveControl, enabled ); 
            setControlEnabled( hiDistParamControl, enabled ); 
        });
    }
}

void AudioPluginAudioProcessorEditor::setControlEnabled( juce::Slider& control, bool enabled )
{
    control.setEnabled( enabled );
    control.setAlpha( enabled ? 1.0f : 0.4f );
}

void AudioPluginAudioProcessorEditor::paint( juce::Graphics& g )
{
    g.fillAll( getLookAndFeel().findColour( juce::ResizableWindow::backgroundColourId ));

    juce::Image background = juce::ImageCache::getFromMemory( BinaryData::background_png, BinaryData::background_pngSize );
    g.drawImage( background, 0, 0, scaledWidth, scaledHeight, 0, 0, Styles::WIDTH, Styles::HEIGHT, false );
    
    int scaledVersionWidth  = static_cast<int>( ceil( Styles::VERSION_WIDTH  / 2 ));
    int scaledVersionHeight = static_cast<int>( ceil( Styles::VERSION_HEIGHT / 2 ));

    juce::Image version = juce::ImageCache::getFromMemory( BinaryData::version_png, BinaryData::version_pngSize );
    g.drawImage(
        version,
        scaledWidth - ( scaledVersionWidth + 21 ), scaledHeight - 34, scaledVersionWidth, scaledVersionHeight,
        0, 0, Styles::VERSION_WIDTH, Styles::VERSION_HEIGHT, false
    );
}

void AudioPluginAudioProcessorEditor::resized()
{
    int loDistControlX = 25;
    int hiDistControlX = 218;
    int loDistSlidersX = loDistControlX - 8;
    int hiDistSlidersX = hiDistControlX - 8;
    int distSectionY   = 98;
    int distSlidersY   = distSectionY + 130;

    linkEnabledControl.setBounds( 159, 319, Styles::CHECKBOX_SIZE, Styles::CHECKBOX_SIZE );
    splitFreqControl.setBounds ( 107, 82, Styles::LARGE_ROTARY_SIZE, Styles::LARGE_ROTARY_SIZE );
    splitModeControl.setBounds( scaledWidth / 2 - Styles::ROTARY_SIZE / 2, 170, Styles::ROTARY_SIZE, Styles::ROTARY_SIZE );
    dryWetControl.setBounds( scaledWidth / 2 - Styles::SLIDER_WIDTH / 2, scaledHeight - 40, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    
    loDistTypeControl.setBounds( loDistControlX, distSectionY, Styles::ROTARY_SIZE, Styles::ROTARY_SIZE );
    loDistInputControl.setBounds( loDistSlidersX, distSlidersY, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    loDistDriveControl.setBounds( loDistSlidersX, distSlidersY + Styles::SLIDER_MARGIN, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    loDistParamControl.setBounds( loDistSlidersX, distSlidersY + Styles::SLIDER_MARGIN * 2, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    
    hiDistTypeControl.setBounds( hiDistControlX, distSectionY, Styles::ROTARY_SIZE, Styles::ROTARY_SIZE );
    hiDistInputControl.setBounds( hiDistSlidersX, distSlidersY, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    hiDistDriveControl.setBounds( hiDistSlidersX, distSlidersY + Styles::SLIDER_MARGIN, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
    hiDistParamControl.setBounds( hiDistSlidersX, distSlidersY + Styles::SLIDER_MARGIN * 2, Styles::SLIDER_WIDTH, Styles::SLIDER_HEIGHT );
}