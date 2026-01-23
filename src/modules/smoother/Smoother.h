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
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Wraps around juce::SmoothedValue to conveniently
 * interpolate a value over a fixed duration to overcome
 * audible glitches when setting it directly.
 */
class Smoother
{
    public:
        Smoother();

        // note provided value is set immediately
        void init( double sampleRate, float durationInSeconds, float value );

        // gets the current interpolated value
        float get();

        // whether interpolation has completed
        bool isDone();

        // gets the next value and advances the interpolation by provided amount
        float peek( int samplesToAdvance );
        
        // set new target value will be interpolated to over duration provided in init()
        void set( float value );

    private:
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> _smoother;
        bool _done;
};