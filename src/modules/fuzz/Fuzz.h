/*
 * Copyright (c) 2024-2025 Igor Zinken https://www.igorski.nl
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

class Fuzz
{
    public:
        Fuzz( float input );

        float getAmount();
        void setAmount( float value );

        float getInputLevel();
        void setInputLevel( float value );

        float getCutOff();
        void setCutOff( float value );
        
        float getThreshold();
        void setThreshold( float value );
        
        void apply( float* channelData, unsigned long bufferSize );

    private:
        float _amount;
        float _input;
        float _cutoffThreshold; // Below this threshold, silence the output
        float _squareWaveThreshold; // Below this threshold, signal is converted to a square wave
};
