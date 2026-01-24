/*
 * Copyright (c) 2026 Igor Zinken https://www.igorski.nl
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
#include "../../Parameters.h"
#include "../../utils/MathUtilities.h"
#include <limits.h>
#include <math.h>

/* constructor / destructor */

BitCrusher::BitCrusher()
{
    setLevel( Parameters::Config::DIST_INPUT_DEF );
    setDownsampling( Parameters::Config::DIST_DRIVE_DEF );
    setAmount( Parameters::Config::DIST_PARAM_DEF );
}

BitCrusher::~BitCrusher()
{
    // nowt...
}

/* public methods */

void BitCrusher::apply( float* channelData, unsigned long bufferSize )
{
    float wrapDrive = _crush * ( MAX_BITS - _bits) / ( MAX_BITS - 1 );
    bool addNoise = _amount > NOISE_THRESHOLD;

    for ( size_t i = 0; i < bufferSize; ++i )
    {
        float input = channelData[ i ];

        int jitter = juce::Random::getSystemRandom().nextInt( int( _jitterAmount * _downsampleBase ) + 1 );
        int effectiveDownsample = juce::jmax( 1, _downsampleBase + jitter );

        if ( ++_sampleCounter >= effectiveDownsample )
        {
            _sampleCounter = 0;

            if ( addNoise ) {
                input += ( juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f ) * _noiseAmount;
            }
            
            // apply bit reduction

            float scaled = input * _levels;
            float crushed = std::floor( scaled ) / _levels;

            // apply wrap drive
            
            crushed *= 1.0f + wrapDrive * 4.0f;
            crushed = std::fmod( crushed + 1.0f, 2.0f ) - 1.0f;

            // storing lastSample as local state will "bleed across channels" (unless Bitcrusher has
            // an instance per channel), but that is fine as it adds another nice layer of unpredictability!!

            _lastSample = crushed;
        }
        channelData[ i ] = _lastSample * _mixLevel;
    }
}

/* setters */

void BitCrusher::setAmount( float value )
{
    _amount = value;
    _bits   = juce::jmap( MathUtilities::inverseNormalize( value ), MIN_BITS, MAX_BITS );
    _levels = std::pow( 2.0f, _bits );
}

void BitCrusher::setDownsampling( float factor )
{
    _crush          = juce::jlimit( 0.0f, 1.0f, factor );
    _jitterAmount   = _crush * 0.6f;
    _noiseAmount    = _crush * 0.02f;
    _downsampleBase = 1 + int( _crush * _crush * 80.0f );
}

void BitCrusher::setLevel( float value )
{
    _mixLevel = juce::jlimit( 0.0f, 1.0f, value );
}
