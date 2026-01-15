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
    setLevel( Parameters::Config::DIST_INPUT_DEF );
    setDrive( Parameters::Config::DIST_DRIVE_DEF );
    setThreshold( Parameters::Config::DIST_DRIVE_DEF );
    setThresholdNegative( Parameters::Config::DIST_PARAM_DEF );
}

/* public methods */

void WaveFolder::apply( float* channelData, unsigned long bufferSize )
{
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * _drive;

        // Hard (asymetric) wavefolding
    
        if ( inputSample > _threshold ) {
            inputSample = _threshold - ( inputSample - _threshold );
        } else if ( inputSample < -_thresholdNegative ) {
            inputSample = -_thresholdNegative - ( inputSample + _thresholdNegative );
        }

        // Alternative: Smooth wavefolding
        // inputSample = std::tanh( inputSample / _threshold ) * _threshold;

        channelData[ i ] = juce::jlimit( -1.0f, 1.0f, inputSample ) * _level;
    }
}

/* setters */

void WaveFolder::setLevel( float value )
{
    _level = value;
}

void WaveFolder::setDrive( float value )
{
    _drive = juce::jmap( value, 1.f, 10.f );
}

void WaveFolder::setThreshold( float value )
{
    _threshold = value;
}

void WaveFolder::setThresholdNegative( float value )
{
    _thresholdNegative = value;
}
