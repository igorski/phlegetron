/*
 * Copyright (c) 2024-2026 Igor Zinken https://www.igorski.nl
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

namespace Parameters {
    static juce::String DRY_WET_MIX   = "dryWetMix";
    static juce::String LINK_ENABLED  = "linkEnabled";
    static juce::String SPLIT_FREQ    = "splitFreq";
    static juce::String SPLIT_MODE    = "splitMode";
    
    // low distortion properties

    static juce::String LO_DIST_TYPE  = "loType";
    static juce::String LO_DIST_INPUT = "loInput";
    static juce::String LO_DIST_DRIVE = "loDrive";
    static juce::String LO_DIST_PARAM = "loParam";

    // high distortion properties (mirrored from low)

    static juce::String HI_DIST_TYPE  = "hiType";
    static juce::String HI_DIST_INPUT = "hiInput";
    static juce::String HI_DIST_DRIVE = "hiDrive";
    static juce::String HI_DIST_PARAM = "hiParam";

    namespace Ranges {
        static float SPLIT_FREQ_MIN = 20.f;
        static float SPLIT_FREQ_MAX = 5000.f;

        static float HARMONIC_COUNT   = 10.f; // specifies how many harmonics of base freq are considered related (1 - 16)
        static float HARMONIC_WIDTH   = 0.3f; // how close a frequency needs to match a harmonic to be considered related ( 0.5 - 10 )
        static float HARMONIC_FALLOFF = 1.0f; // 0 = pure fundamental, 1 = natural harmonic spread, 2 = high harmonic emphasis
    }

    namespace FFT {
        static const int ORDER = 11; // 2048-point FFT

        // these are deduced from above ORDER
        static const unsigned long FFT_SIZE    = 1 << Parameters::FFT::ORDER;
        static const unsigned long DOUBLE_SIZE = Parameters::FFT::FFT_SIZE * 2;
        static const unsigned long HOP_SIZE    = Parameters::FFT::FFT_SIZE / 2;
    }

    enum class SplitMode {
        EQ = 0,
        Harmonic,
    };

    enum class DistortionType {
        Off = 0,
        WaveShaper,
        WaveFolder,
        Fuzz,
        BitCrusher,
    };

    namespace Config {
        static int DIST_TYPE_DEF_LO = static_cast<int>( DistortionType::WaveShaper );
        static int DIST_TYPE_DEF_HI = static_cast<int>( DistortionType::WaveFolder );
        static float DIST_INPUT_DEF = 1.f;
        static float SPLIT_FREQ_DEF = 440.f;
        static float DIST_DRIVE_DEF = 0.5f;
        static float DIST_PARAM_DEF = 0.70f;
    }
}
