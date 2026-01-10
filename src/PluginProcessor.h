/*
 * Copyright (c) 2024-2025 Igor Zinken https://www.igorski.nl
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
#include "modules/fuzz/Fuzz.h"
#include "modules/wavefolder/Wavefolder.h"
#include "Parameters.h"
#include "ParameterListener.h"
#include "ParameterSubscriber.h"

class AudioPluginAudioProcessor final : public juce::AudioProcessor, ParameterSubscriber
{
    public:
        AudioPluginAudioProcessor();
        ~AudioPluginAudioProcessor() override;

        /* configuration */

        const juce::String getName() const override;
        bool isBusesLayoutSupported( const BusesLayout& layouts ) const override;

        bool acceptsMidi() const override;
        bool producesMidi() const override;
        bool isMidiEffect() const override;
        double getTailLengthSeconds() const override;

        /* programs */

        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram( int index ) override;
        const juce::String getProgramName( int index ) override;
        void changeProgramName( int index, const juce::String& newName ) override;

        /* resource management */

        void prepareToPlay( double sampleRate, int samplesPerBlock ) override;
        void releaseResources() override;

        /* rendering */

        void processBlock( juce::AudioBuffer<float>&, juce::MidiBuffer& ) override;
        using AudioProcessor::processBlock;

        /* automatable parameters */

        juce::AudioProcessorValueTreeState parameters;
        ParameterListener parameterListener;
        void updateParameters() override;

        static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
        {
            std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::DRY_WET_MIX, "Dry/wet mix", 0.f, 1.f, 1.f )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterBool>( Parameters::SPLIT_ENABLED, "Split enabled", true )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterBool>( Parameters::SPLIT_MODE, "Split mode", false )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::SPLIT_FREQ,  "Split frequency",
                    Parameters::Ranges::SPLIT_FREQ_MIN, Parameters::Ranges::SPLIT_FREQ_MAX, Parameters::Config::SPLIT_FREQ_DEF
                )
            );

            // low band distortion
            params.push_back(
                std::make_unique<juce::AudioParameterBool>( Parameters::LO_DIST_TYPE, "Low type", false )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::LO_DIST_INPUT, "Low Input level", 0.f, 1.f, 0.5f )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::LO_DIST_CUTOFF, "Low Cutoff threshold", 0.f, 1.f, Parameters::Config::DIST_CUT_THRESH_DEF )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::LO_DIST_THRESH, "Low Threshold", 0.f, 1.f, Parameters::Config::DIST_THRESH_DEF )
            );

            // high band distortion

            params.push_back(
                std::make_unique<juce::AudioParameterBool>( Parameters::HI_DIST_TYPE, "Hi type", true )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::HI_DIST_INPUT, "Hi Input level", 0.f, 1.f, 0.5f )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::HI_DIST_CUTOFF, "Hi Cutoff threshold", 0.f, 1.f, Parameters::Config::DIST_CUT_THRESH_DEF )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::HI_DIST_THRESH, "Hi Threshold", 0.f, 1.f, Parameters::Config::DIST_THRESH_DEF )
            );
            
            return { params.begin(), params.end() };
        }

        /* editor */
        
        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override;

        /* persistence */

        void getStateInformation( juce::MemoryBlock& destData ) override;
        void setStateInformation( const void* data, int sizeInBytes ) override;
        
        /* runtime state */

        bool alignWithSequencer( juce::Optional<juce::AudioPlayHead::PositionInfo> positionInfo );
        
    private:
        juce::OwnedArray<juce::IIRFilter> lowPassFilters;
        juce::OwnedArray<juce::IIRFilter> highPassFilters;
        Fuzz lowFuzz;
        Fuzz hiFuzz;
        WaveFolder lowWaveFolder;
        WaveFolder hiWaveFolder;

        float ATTENUATION_FACTOR = 0.5f; // two channels (low and hi)
        
        // FFT

        struct ChannelState
        {
            std::vector<float> inputBuffer;
            std::vector<float> outputBuffer;
            int writePos = 0;
            bool initialised = false;
        };
        
        double _sampleRate;
        
        bool isPlaying  = false;
        int timeSigNumerator   = 4;
        int timeSigDenominator = 4;
        double tempo = 120.0;
        
        // parameters

        std::atomic<float>* dryWetMix;
        std::atomic<float>* splitEnabled;
        std::atomic<float>* splitFreq;
        std::atomic<float>* splitMode;
        std::atomic<float>* loDistType;
        std::atomic<float>* loDistInputLevel;
        std::atomic<float>* loDistThreshold;
        std::atomic<float>* loDistCutoffThreshold;
        std::atomic<float>* hiDistType;
        std::atomic<float>* hiDistInputLevel;
        std::atomic<float>* hiDistThreshold;
        std::atomic<float>* hiDistCutoffThreshold;

        inline bool floatToBool( const float value ) {
            return value >= 0.5f;
        }

        inline void applyDistortion(
            juce::AudioBuffer<float>& lowBuffer, juce::AudioBuffer<float>& highBuffer, const int channelNum
        ) {
            applyDistortion(
                lowBuffer.getWritePointer( channelNum ),
                highBuffer.getWritePointer( channelNum ),
                static_cast<unsigned long>( lowBuffer.getNumSamples() ),
                static_cast<unsigned long>( highBuffer.getNumSamples() )
            );
        }

        inline void applyDistortion(
            float* lowChannelData, float* highChannelData, unsigned long lowChannelSize, unsigned long highChannelSize
        ) {
            bool splitProcessing  = floatToBool( *splitEnabled );
            
            if ( floatToBool( *loDistType )) {
                lowWaveFolder.apply( lowChannelData, lowChannelSize );
                if ( !splitProcessing ) {
                    lowWaveFolder.apply( highChannelData, highChannelSize );
                }
            } else {
                lowFuzz.apply( lowChannelData, lowChannelSize );
                if ( !splitProcessing ) {
                    lowFuzz.apply( highChannelData, highChannelSize );
                }
            }

            if ( splitProcessing ) {
                if ( floatToBool( *hiDistType )) {
                    hiWaveFolder.apply( highChannelData, highChannelSize );
                } else {
                    hiFuzz.apply( highChannelData, highChannelSize );
                }
            }
        }
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioPluginAudioProcessor )
};
