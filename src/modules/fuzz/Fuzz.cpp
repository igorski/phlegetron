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
#include "Fuzz.h"
#include "../../Parameters.h"

// constructor

Fuzz::Fuzz( float input )
{
    setInputLevel( input );
    setCutOff( Parameters::Config::DIST_CUT_THRESH_DEF );
    setThreshold( Parameters::Config::DIST_THRESH_DEF );

    _amount = 10.f; // provide clipping effect to the input
}

/* public methods */

void Fuzz::apply( float* channelData, unsigned long bufferSize )
{
    for ( int i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * _input;

        // Absolute value of the sample to determine the signal level
        float absSample = std::abs( inputSample );

        // Apply fuzz distortion when the signal is above the squareWaveThreshold
        if ( absSample > _squareWaveThreshold ) {
            // Hard clip to fuzz
            inputSample = juce::jlimit( -1.0f, 1.0f, _amount * inputSample );
        } else if ( absSample > _cutoffThreshold ) {
            // Convert signal to a square wave for low signal levels
            inputSample = inputSample > 0.0f ? 1.0f : -1.0f;
        } else {
            // Below the cutoff threshold, output silence
            inputSample = 0.0f;
        }
        channelData[ i ] = inputSample;
    }
}

/* getters / setters */

float Fuzz::getAmount()
{
    return _amount;
}

void Fuzz::setAmount( float value )
{
    _amount = value;
}

float Fuzz::getInputLevel()
{
    return _input;
}

void Fuzz::setInputLevel( float value )
{
    _input = value;
}

float Fuzz::getCutOff()
{
    return _cutoffThreshold;
}

void Fuzz::setCutOff( float value )
{
    _cutoffThreshold = value;
}


float Fuzz::getThreshold()
{
    return _squareWaveThreshold;
}

void Fuzz::setThreshold( float value )
{
    _squareWaveThreshold = value;
}
