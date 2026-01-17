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
#include "Styles.h"

class ThinRotaryLookAndFeel : public juce::LookAndFeel_V4
{
    public:
        void drawRotarySlider(
            juce::Graphics& g, int x, int y, int width, int height,
            float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider
        ) override {
            juce::ignoreUnused( slider );

            auto bounds = juce::Rectangle<float>( x, y, width, height ).reduced( 10 );
            auto radius = juce::jmin( bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto centre = bounds.getCentre();

            juce::PathStrokeType stroke( strokeSize, juce::PathStrokeType::curved, juce::PathStrokeType::rounded );

            // background
            juce::Path backgroundArc;
            backgroundArc.addCentredArc(
                centre.x, centre.y, radius, radius,
                0.0f, rotaryStartAngle, rotaryEndAngle, true
            );
            g.setColour( juce::Colour( Styles::BACKGROUND_COLOR ));
            g.strokePath( backgroundArc, stroke );

            // highlight
            auto angle = rotaryStartAngle + sliderPosProportional * ( rotaryEndAngle - rotaryStartAngle );

            juce::Path valueArc;
            valueArc.addCentredArc(
                centre.x, centre.y, radius, radius,
                0.0f, rotaryStartAngle, angle, true
            );
            g.setColour( juce::Colour( Styles::HIGHLIGHT_COLOR ));
            g.strokePath( valueArc, stroke );
        }

        void setStrokeSize( float value ) {
            strokeSize = value;
        }

    private:
        float strokeSize = 4.0f;
};