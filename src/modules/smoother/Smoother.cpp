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
#include "Smoother.h"

/* constructor */

Smoother::Smoother()
{
    _done = false;
}

/* public methods */

void Smoother::init( double sampleRate, float durationInSeconds, float value )
{
    _done = false;
    _smoother.reset( sampleRate, durationInSeconds );
    _smoother.setCurrentAndTargetValue( value );
}

float Smoother::get()
{
    return _smoother.getCurrentValue();
}

bool Smoother::isDone()
{
    if ( _done ) {
        return true;
    }
    _done = !_smoother.isSmoothing();

    return _done;
}

float Smoother::peek( int samplesToAdvance )
{
    const float output = _smoother.getNextValue();
    _smoother.skip( samplesToAdvance );

    return output;
}

void Smoother::set( float value )
{
    _done = false;
    _smoother.setTargetValue( value );
}