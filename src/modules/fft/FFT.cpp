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
    _fft = new juce::dsp::FFT( Parameters::FFT::ORDER );
}

FFT::~FFT()
{
    delete _fft;
}

/* public methods */

void FFT::forward( float* data )
{
    _fft->performRealOnlyForwardTransform( data );
}

void FFT::inverse( /*const std::vector<std::complex<float>>& src,*/ std::vector<float>& dst )
{
    // for ( size_t i = 0; i < Parameters::FFT::HOP_SIZE; ++i ) {
    //     dst[ 2 * i ]     = src[ i ].real();
    //     dst[ 2 * i + 1 ] = src[ i ].imag();
    // }
    _fft->performRealOnlyInverseTransform( dst.data() );
}
