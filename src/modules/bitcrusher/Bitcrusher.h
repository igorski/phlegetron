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

class BitCrusher
{
    static constexpr float MAX_BITS = 16.f;
    static constexpr float MIN_BITS = 1.f;
    static constexpr float NOISE_THRESHOLD = 0.5f;

    public:
        BitCrusher();
        ~BitCrusher();

        void apply( float* channelData, unsigned long bufferSize );

        void setAmount( float value );
        void setDownsampling( float value );
        void setLevel( float value );

    private:
        float _bits; // amount scaled within 1 - 16 range
        float _mixLevel;
        float _amount;
        float _crush;
        float _levels;
        int _downsampleBase;
        float _jitterAmount;
        float _noiseAmount;
        int _sampleCounter = 0;
        float _lastSample = 0.0f;
};
