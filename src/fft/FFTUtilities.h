/*
 * Copyright (c) 2025 Igor Zinken https://www.igorski.nl
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

#include <vector>

class FFTUtilities
{
    public:
        static float detectPitch( const float* buffer, int numSamples, double sampleRate )
        {
            int maxLag    = numSamples / 2;
            float bestLag = 0;
            float maxCorr = 0.0f;

            for ( int lag = 20; lag < maxLag; ++lag )  // skip very low lags
            {
                float corr = 0.0f;
                for ( int i = 0; i < numSamples - lag; ++i ) {
                    corr += buffer[ i ] * buffer[ i + lag ];
                }

                if ( corr > maxCorr ) {
                    maxCorr = corr;
                    bestLag = lag;
                }
            }
            return ( bestLag > 0 ) ? sampleRate / bestLag : 0.f;
        }

        static std::vector<int> getHarmonicBins( float fundamental, int fftSize, double sampleRate, int numHarmonics )
        {
            std::vector<int> bins;
            
            if ( fundamental <= 0.0f ) {
                return bins;
            }
            
            float binSize = sampleRate / fftSize;
            
            for ( int n = 1; n <= numHarmonics; ++n ) {
                int bin = static_cast<int>( round(( fundamental * n ) / binSize ));
                if ( bin < fftSize / 2 ) {// only positive frequencies
                    bins.push_back( bin );
                }
            }
            return bins;
        }

        static std::vector<int> computeHarmonicBins(
            float fundamentalFreq, int fftSize, double sampleRate, int numHarmonics
        ) {
            std::vector<int> bins;
            float binSize = sampleRate / fftSize;

            for (int n = 1; n <= numHarmonics; ++n)
            {
                int bin = static_cast<int>(std::round(fundamentalFreq * n / binSize));
                if (bin < fftSize / 2) // only positive frequencies
                    bins.push_back(bin);
            }
            return bins;
        }
};