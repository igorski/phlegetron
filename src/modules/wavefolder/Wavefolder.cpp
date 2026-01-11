/*
 * Copyright (c) 2025-2026 Igor Zinken https://www.igorski.nl
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
#include "Wavefolder.h"
#include "../../Parameters.h"

// constructor

WaveFolder::WaveFolder()
{
    setInputLevel( 1.0f );
    setThreshold( Parameters::Config::DIST_THRESH_DEF );
}

/* public methods */

void WaveFolder::apply( float* channelData, unsigned long bufferSize )
{
    float gain = _input * 10.f;

    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * gain;

        // Hard wavefolding
    
        if ( inputSample > _threshold ) {
            inputSample = _threshold - ( inputSample - _threshold );
        } else if ( inputSample < -_thresholdNegative ) {
            inputSample = -_thresholdNegative - ( inputSample + _thresholdNegative );
        }

        // Alternative: Smooth wavefolding
        // inputSample = std::tanh( inputSample / _threshold ) * _threshold;

        inputSample = juce::jlimit( -1.0f, 1.0f, inputSample );

        channelData[ i ] = inputSample;
    }
}

/* getters / setters */

float WaveFolder::getInputLevel()
{
    return _input;
}

void WaveFolder::setInputLevel( float value )
{
    _input = value;
}

float WaveFolder::getThreshold()
{
    return _threshold;
}

void WaveFolder::setThreshold( float value )
{
    _threshold = value;
}

float WaveFolder::getThresholdNegative()
{
    _thresholdNegative;
}

void WaveFolder::setThresholdNegative( float value )
{
    _thresholdNegative = value;
}
