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
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class WaveFolder
{
    public:
        WaveFolder();

        void setLevel( float value );
        void setDrive( float value );
        void setThreshold( float value );

        // we could consider supplying a different value for the threshold
        // to allow for asymmetric wave folding
        // void setThresholdNegative( float value );
        
        void apply( float* channelData, unsigned long bufferSize );

    private:
        // amount of times we fold the waveform over itself
        // when it exceeds the threshold (increases harmonic complexity)

        static constexpr float FOLDING_MULTIPLIER = 2.0f;

        float _level;
        float _drive;
        float _fold;
        float _threshold;
        // float _thresholdNegative;
};
