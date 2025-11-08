/*
 * Copyright (c) 2024 Igor Zinken https://www.igorski.nl
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
    static juce::String DIST_INPUT      = "distInput";
    static juce::String DIST_THRESHOLD  = "distThreshold";
    static juce::String DIST_CUT_THRESH = "distCutoffThreshold"; 

    namespace Ranges {
        static float LOW_BAND_MIN = 20.f;
        static float LOW_BAND_MAX = 500.f;
        static float MID_BAND_MIN = 500.f;
        static float MID_BAND_MAX = 3000.f;
        static float HI_BAND_MIN  = 5000.f;
        static float HI_BAND_MAX  = 11025.f;
    }

    namespace Config {
        static float LOW_BAND_DEF = 200.f;
        static float MID_BAND_DEF = 1000.f;
        static float HI_BAND_DEF  = 5000.f;

        static float DIST_CUT_THRESH_DEF = 0.02f;
        static float DIST_THRESH_DEF     = 0.1f;
    }
}
