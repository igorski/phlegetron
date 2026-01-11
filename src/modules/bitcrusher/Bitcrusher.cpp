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
#include "Bitcrusher.h"
#include "../../utils/MathUtilities.h"
#include <limits.h>
#include <math.h>

/* constructor / destructor */

BitCrusher::BitCrusher()
{
    setAmount( 0.f );
    setInputLevel ( 1.f );
    setOutputLevel( 1.f );
}

BitCrusher::~BitCrusher()
{

}

/* public methods */

void BitCrusher::apply( float* channelData, unsigned long bufferSize )
{
    // sound should not be crushed ? do nothing
    if ( _bits == 16 ) {
        return;
    }

    int bitsPlusOne = _bits + 1;

    for ( size_t i = 0; i < bufferSize; ++i )
    {
        short input = ( short ) (( channelData[ i ] * _inputMix ) * SHRT_MAX );
        short prevent_offset = ( short )( -1 >> bitsPlusOne );
        input &= ( -1 << ( 16 - _bits ));
        channelData[ i ] = (( input + prevent_offset ) * _outputMix ) / SHRT_MAX;
    }
}

/* setters */

void BitCrusher::setAmount( float value )
{
    // invert the range 0 == max bits (no distortion), 1 == min bits (severely distorted)
    _amount = abs( value - 1.f );

    calcBits();
}

void BitCrusher::setInputLevel( float value )
{
    _inputMix = juce::jlimit( 0.0f, 1.0f, value );
}

void BitCrusher::setOutputLevel( float value )
{
    _outputMix = juce::jlimit( 0.0f, 1.0f, value );
}

/* private methods */

void BitCrusher::calcBits()
{
    // scale float to 1 - 16 bit range
    _bits = ( int ) floor( MathUtilities::scale( _amount, 1, 15 )) + 1;
}
