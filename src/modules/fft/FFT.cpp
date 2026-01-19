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
#include "FFT.h"

/* constructor / destructor */

FFT::FFT()
{
    fftTime.resize(( size_t ) Parameters::FFT::DOUBLE_SIZE );
    window.resize( Parameters::FFT::SIZE );

    for ( size_t n = 0; n < Parameters::FFT::SIZE; ++n ) {
        window[ n ] = 0.5f - 0.5f * std::cos( 2.f * juce::MathConstants<float>::pi * n / Parameters::FFT::SIZE );
    }

    harmonics.reserve(( size_t ) Parameters::Ranges::HARMONIC_COUNT );
    harmonicMask.reserve(( size_t ) Parameters::FFT::HOP_SIZE );

    _fft = new juce::dsp::FFT( Parameters::FFT::ORDER );
}

FFT::~FFT()
{
    delete _fft;
}

/* public methods */

void FFT::update( double sampleRate )
{
    _sampleRate = ( float ) sampleRate;
    _nyquist = ( float ) _sampleRate * 0.5f;
}

void FFT::calculateHarmonics( float frequency )
{
    if ( juce::approximatelyEqual( frequency, _lastFreq )) {
        return; // no need to recalculate
    }
    harmonics.clear();
    
    for ( size_t h = 1; h <= Parameters::Ranges::HARMONIC_COUNT; ++h )
    {
        float freq = h * frequency;
        if ( freq >= _nyquist ) {
            break;
        }
        Harmonic harm;
        harm.freq = freq;
        harm.widthHz = freq * Parameters::Ranges::HARMONIC_WIDTH;
        harm.weight = 1.0f / std::pow(( float ) h, Parameters::Ranges::HARMONIC_FALLOFF ); 
        harmonics.push_back( harm );
    }

    // calculate harmonic mask

    for ( size_t bin = 0; bin < Parameters::FFT::HOP_SIZE; ++bin )
    {
        float binFreq = ( float ) bin * ( float ) _sampleRate / ( float ) Parameters::FFT::SIZE;
        float maskA = 0.0f;

        for ( Harmonic harmonic : harmonics )
        {
            float distance = std::abs( binFreq - harmonic.freq );
            float norm = distance / harmonic.widthHz;

            if ( norm < 1.0f )
            {
                // option A (soft triangular mask)
                float contribution = harmonic.weight * ( 1.0f - norm );

                // option B (smoother mask curve for less ringing)
                // float t = 1.0f - norm;
                // float smooth = t * t * (3.0f - 2.0f * t);
                // float contribution = harmonic.weight * smooth;

                maskA = std::max( maskA, contribution );
            }
        }
        harmonicMask[ bin ] = juce::jlimit( 0.0f, 1.0f, maskA );
    }
    _lastFreq = frequency;
}

void FFT::split( const std::vector<float>& inputBuffer, std::vector<float>& specA, std::vector<float>& specB ) {

    // apply window to overcome spectral leakage

    for ( size_t i = 0; i < Parameters::FFT::SIZE; ++i ) {
        fftTime[ i ] = inputBuffer[ i ] * window[ i ];
    }

    // apply forward transform

    _fft->performRealOnlyForwardTransform( fftTime.data() );

    // split spectrum by harmonic proximity

    for ( size_t bin = 0; bin < Parameters::FFT::HOP_SIZE; ++bin )
    {
        float maskA = harmonicMask[ bin ];
        float maskB = 1.0f - maskA;

        size_t realIndex = 2 * bin;
        size_t imagIndex = 2 * bin + 1;

        const float real = fftTime[ realIndex ];
        const float imag = fftTime[ imagIndex ];

        specA[ realIndex ] = real * maskA;
        specA[ imagIndex ] = imag * maskA;

        specB[ realIndex ] = real * maskB;
        specB[ imagIndex ] = imag * maskB;
    }

    // apply inverse transform

    _fft->performRealOnlyInverseTransform( specA.data() );
    _fft->performRealOnlyInverseTransform( specB.data() );
}

void FFT::sum( std::vector<float>& outputBuffer, std::vector<float>& specA, std::vector<float>& specB )
{
    for ( size_t i = 0; i < Parameters::FFT::SIZE; ++i ) {
        outputBuffer[ i ] += ( specA[ i ] + specB[ i ]) * window[ i ];
    }
}
