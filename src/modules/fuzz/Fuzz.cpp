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
#include "Fuzz.h"
#include "../../Parameters.h"

// constructor

Fuzz::Fuzz()
{
    setInputLevel( Parameters::Config::DIST_INPUT_DEF );
    setCutOff( Parameters::Config::DIST_PARAM_DEF );
    setThreshold( Parameters::Config::DIST_DRIVE_DEF );
    setDrive( 0.22f );
}

/* public methods */

void Fuzz::apply( float* channelData, unsigned long bufferSize )
{
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * _input;
        float absSample   = std::abs( inputSample ); // level used in threshold comparison

        if ( absSample > _squareWaveThreshold )
        {
            // signal is above threshold, hard clip it!
            inputSample = juce::jlimit( -1.0f, 1.0f, _drive * inputSample );
        }
        else if ( absSample > _cutoffThreshold )
        {
            // when between square wave and cutoff thresholds, the signal should become a square wave
            inputSample = inputSample > 0.0f ? 1.0f : -1.0f;
        }
        else {
            // signal is below the cutoff threshold, make it silent
            inputSample = 0.0f;
        }
        channelData[ i ] = inputSample;
    }
}

/*  setters */

void Fuzz::setDrive( float value )
{
    _drive = juce::jmap( value, 1.f, 10.f );
}

void Fuzz::setInputLevel( float value )
{
    _input = value;
}

void Fuzz::setCutOff( float value )
{
    _cutoffThreshold = value;
}

void Fuzz::setThreshold( float value )
{
    _squareWaveThreshold = value;
}
