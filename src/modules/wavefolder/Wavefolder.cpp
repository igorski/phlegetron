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
    float foldAmount = juce::jmax( 0.001f, _threshold / _fold );
    float range = FOLDING_MULTIPLIER * foldAmount;
    
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float inputSample = channelData[ i ] * _level;

        float wrapped = std::fmod( inputSample + foldAmount, range );
        if ( wrapped < 0.0f ) {
            wrapped += range;
        }
        float folded = std::abs( wrapped - foldAmount ) - foldAmount;

        // apply drive to the folded signal

        float outputSample = std::tanh( folded * _drive ) / std::tanh( _drive );

        channelData[ i ] = outputSample;
    }
}

/* setters */

void WaveFolder::setLevel( float value )
{
    _level = value;
}

void WaveFolder::setDrive( float value )
{
    // we control both fold and drive with a single value
    _fold  = FOLD_MIN + std::pow( value, 1.8f ) * ( FOLD_MAX - FOLD_MIN );
    _drive = DRIVE_MIN + std::pow( value, 2.2f ) * ( DRIVE_MAX  - DRIVE_MIN );
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
