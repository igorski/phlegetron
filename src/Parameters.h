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
    static juce::String DRY_WET_MIX     = "dryWetMix";
    static juce::String SPLIT_ENABLED   = "splitEnabled";
    static juce::String SPLIT_FREQ      = "splitFreq";
    static juce::String SPLIT_MODE      = "splitMode";
    
    // low distortion properties

    static juce::String LO_DIST_TYPE   = "loType";
    static juce::String LO_DIST_INPUT  = "loInput";
    static juce::String LO_DIST_THRESH = "loThreshold"; // @todo : "drive" ?
    static juce::String LO_DIST_CUTOFF = "loCutoff";

    // high distortion properties (mirrored from low)

    static juce::String HI_DIST_TYPE   = "hiType";
    static juce::String HI_DIST_INPUT  = "hiInput";
    static juce::String HI_DIST_THRESH = "hiThreshold"; // @todo : "drive" ?
    static juce::String HI_DIST_CUTOFF = "hiCutoff";

    namespace Ranges {
        static float SPLIT_FREQ_MIN = 20.f;
        static float SPLIT_FREQ_MAX = 5000.f;
    }

    enum class SplitMode {
        EQ = 0,
        Harmonic,
    };

    enum class DistortionType {
        Fuzz = 0,
        WaveFolder,
    };

    namespace Config {
        static float SPLIT_FREQ_DEF      = 110.f;
        static float DIST_CUT_THRESH_DEF = 0.02f;
        static float DIST_THRESH_DEF     = 0.1f;
    }
}
