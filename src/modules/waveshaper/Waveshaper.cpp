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
#include <cmath>

// constructor

WaveShaper::WaveShaper()
{
    setAmount( Parameters::Config::DIST_DRIVE_DEF );
    setOutputLevel( Parameters::Config::DIST_INPUT_DEF );
}

/* public methods */

void WaveShaper::apply( float* channelData, unsigned long bufferSize )
{
    for ( size_t i = 0; i < bufferSize; ++i )
    {
        auto input = channelData[ i ];
        channelData[ i ] =  (( 1.0f + _multiplier ) * input / ( 1.0f + _multiplier * std::abs( input ))) * _level;
    }
}

/* getters / setters */

float WaveShaper::getAmount()
{
    return _amount;
}

void WaveShaper::setAmount( float value )
{
    _amount     = value;
    _multiplier = 2.0f * _amount / ( 1.0f - fmin( 0.99999f, _amount ));
}

float WaveShaper::getOutputLevel()
{
    return _level;
}

void WaveShaper::setOutputLevel( float value )
{
    _level = value;
}
