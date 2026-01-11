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
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class BitCrusher
{
    public:
        BitCrusher();
        ~BitCrusher();

        void apply( float* channelData, unsigned long bufferSize );

        void setAmount( float value ); // range between -1 to +1
        void setInputLevel( float value );
        void setOutputLevel( float value );

    private:
        int _bits; // we scale the amount to integers in the 1-16 range
        float _amount;
        float _inputMix;
        float _outputMix;

        void calcBits();
};
