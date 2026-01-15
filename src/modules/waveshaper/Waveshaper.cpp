/*
 * Copyright (c) 2013-2026 Igor Zinken https://www.igorski.nl
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
#include "Waveshaper.h"
#include "../../Parameters.h"
#include "../../utils/MathUtilities.h"
#include <cmath>

// constructor

WaveShaper::WaveShaper()
{
    setAmount( Parameters::Config::DIST_DRIVE_DEF );
    setShape( Parameters::Config::DIST_PARAM_DEF );
    setOutputLevel( Parameters::Config::DIST_INPUT_DEF );
}

/* public methods */

void WaveShaper::apply( float* channelData, unsigned long bufferSize )
{
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        auto input = channelData[ i ];
        
        float sign = std::copysign( 1.0f, input );
        input = sign * std::pow( std::abs( input ), _shape );

        float shaped = (( 1.0f + _multiplier ) * input ) / ( 1.0f + _multiplier * std::abs( input ));

        channelData[ i ] = shaped * _level;
    }
}

/* setters */

void WaveShaper::setAmount( float value )
{
    _amount     = value;
    _multiplier = 2.0f * _amount / ( 1.0f - fmin( 0.99999f, _amount ));
}

void WaveShaper::setShape( float value )
{
    _shape = juce::jmap( MathUtilities::inverseNormalize( value ), 0.25f, 4.0f );   
}

void WaveShaper::setOutputLevel( float value )
{
    _level = value;
}
