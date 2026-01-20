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
#include "../../utils/MathUtilities.h"

// constructor

WaveFolder::WaveFolder()
{
    setLevel( Parameters::Config::DIST_INPUT_DEF );
    setDrive( Parameters::Config::DIST_DRIVE_DEF );
    setThreshold( Parameters::Config::DIST_PARAM_DEF );
    // setThresholdNegative( Parameters::Config::DIST_PARAM_DEF );
}

/* public methods */

void WaveFolder::apply( float* channelData, unsigned long bufferSize )
{
    float period = FOLDING_MULTIPLIER * _threshold;

    const float makeupGain = 1.0f / std::sqrt( _drive );
// float softness = juce::jmap( _drive, 0.0f, 1.0f, 0.8f, 0.3f);
    
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * _drive;

        float wrapped = std::fmod( inputSample + _threshold, period );
        if ( wrapped < 0.0f ) {
            wrapped += period;
        }
        
        float folded = std::abs( wrapped - _threshold ) - _threshold;

        // float folded = wrapped;
        // if ( wrapped > _threshold ) {
        //     folded = _threshold - ( inputSample - _threshold );
        // } else if ( wrapped < -_threshold ) {
        //     folded = -_threshold - ( inputSample + _threshold );
        // }

        // Hard (asymetric) wavefolding
    
        // if ( inputSample > _threshold ) {
        //     folded = _threshold - ( inputSample - _threshold );
        // } else if ( inputSample < -_thresholdNegative ) {
        //     folded = -_thresholdNegative - ( inputSample + _thresholdNegative );
        // }

        // Alternative: Smooth wavefolding
        // folded = std::tanh( inputSample / _threshold ) * _threshold;

        // Soft saturation for musicality
        // channelData[ i ] = (std::tanh(folded * softness) / std::tanh(softness)) * _level;

        channelData[ i ] = folded * makeupGain * _level;
    }
}

/* setters */

void WaveFolder::setLevel( float value )
{
    _level = value;
}

void WaveFolder::setDrive( float value )
{
    _drive = std::pow( 5.0f, value * 2.0f );//juce::jmap( value, 1.f, 20.f );
}

void WaveFolder::setThreshold( float value )
{
    _threshold = juce::jmap(
        MathUtilities::inverseNormalize( value ), 0.1f, 0.5f
    );
}

// void WaveFolder::setThresholdNegative( float value )
// {
//     _thresholdNegative = value;
// }
