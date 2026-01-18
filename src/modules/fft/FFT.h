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
#include <juce_dsp/juce_dsp.h>
#include "../../Parameters.h"

class FFT
{
    public:
        FFT();
        ~FFT();
        
        void update( double sampleRate );
        void calculateHarmonics( float frequency );
        void split( const std::vector<float>& inputBuffer, std::vector<float>& specA, std::vector<float>& specB );
        void sum( std::vector<float>& outputBuffer, std::vector<float>& specA, std::vector<float>& specB );
        
    private:
        juce::dsp::FFT* _fft;
        std::vector<float> fftTime;
        std::vector<float> window;

        struct Harmonic
        {
            float freq;
            float widthHz;
            float weight;
        };

        std::vector<Harmonic> harmonics;
        std::vector<float> harmonicMask;
        float _sampleRate = 44100.f;
        float _nyquist = 22050.f;
        float _lastFreq = 0.f;
};