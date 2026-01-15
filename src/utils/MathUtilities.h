/*
 * Copyright (c) 2018-2020 Igor Zinken https://www.igorski.nl
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
#include <algorithm>

class MathUtilities
{
    public:
        /**
         * inverts a 0 - 1 normalized min-to-max value to have 0 be the max and 1 the min
         */
        static inline float inverseNormalize( float value ) {
            return ( 1.f - value ) / 1.f;
        }

        /**
         * ensures provided value doesn't exceed signal headroom
         */
        static inline float clamp( float value ) {
            return juce::jlimit( -1.0f, 1.0f, value );
        }
};
