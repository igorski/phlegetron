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
#include <juce_dsp/juce_dsp.h>
#include "modules/bitcrusher/Bitcrusher.h"
#include "modules/fuzz/Fuzz.h"
#include "modules/wavefolder/Wavefolder.h"
#include "modules/waveshaper/Waveshaper.h"
#include "utils/ParameterUtilities.h"
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
                std::make_unique<juce::AudioParameterBool>( Parameters::LINK_ENABLED, "Link enabled", false )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterChoice>( Parameters::SPLIT_MODE, "Split mode", ParameterUtilities::getSplitModeNames(), 0 )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::SPLIT_FREQ,  "Split frequency",
                    Parameters::Ranges::SPLIT_FREQ_MIN, Parameters::Ranges::SPLIT_FREQ_MAX, Parameters::Config::SPLIT_FREQ_DEF
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>( Parameters::DRY_WET_MIX, "Dry/wet mix", 0.f, 1.f, 1.f )
            );

            // low band distortion
            params.push_back(
                std::make_unique<juce::AudioParameterChoice>(
                    Parameters::LO_DIST_TYPE, "Low distortion type",
                    ParameterUtilities::getDistortionTypeNames(), Parameters::Config::DIST_TYPE_DEF_LO
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::LO_DIST_INPUT, "Low Input level", 0.f, 1.f, Parameters::Config::DIST_INPUT_DEF
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::LO_DIST_DRIVE, "Low drive", 0.f, 1.f, Parameters::Config::DIST_DRIVE_DEF
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::LO_DIST_PARAM, "Low param", 0.f, 1.f, Parameters::Config::DIST_PARAM_DEF
                )
            );

            // high band distortion

            params.push_back(
                std::make_unique<juce::AudioParameterChoice>(
                    Parameters::HI_DIST_TYPE, "Hi distortion type",
                    ParameterUtilities::getDistortionTypeNames(), Parameters::Config::DIST_TYPE_DEF_HI
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::HI_DIST_INPUT, "Hi Input level", 0.f, 1.f, Parameters::Config::DIST_INPUT_DEF
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::HI_DIST_DRIVE, "Hi drive", 0.f, 1.f, Parameters::Config::DIST_DRIVE_DEF
                )
            );
            params.push_back(
                std::make_unique<juce::AudioParameterFloat>(
                    Parameters::HI_DIST_PARAM, "Hi param", 0.f, 1.f, Parameters::Config::DIST_PARAM_DEF
                )
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

        // crossover filter processing

        static constexpr int MAX_CHANNELS = 2;
        static constexpr float ATTENUATION_FACTOR = 1.f / MAX_CHANNELS;
        
        juce::dsp::LinkwitzRileyFilter<float> lowPass[ MAX_CHANNELS ];
        juce::dsp::LinkwitzRileyFilter<float> highPass[ MAX_CHANNELS ];
        juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> splitFreqSmoothed;
        std::vector<float> lowBuffer;
        std::vector<float> highBuffer;

        inline void prepareCrossoverFilter(
            juce::dsp::LinkwitzRileyFilter<float> &filter, juce::dsp::LinkwitzRileyFilterType type, float frequency
        ) {
            filter.reset();
            filter.setCutoffFrequency( frequency );
            filter.setType( type );
        }

        // effects modules

        BitCrusher lowBitCrusher;
        BitCrusher hiBitCrusher;
        Fuzz lowFuzz;
        Fuzz hiFuzz;
        WaveFolder lowWaveFolder;
        WaveFolder hiWaveFolder;
        WaveShaper lowWaveShaper;
        WaveShaper hiWaveShaper;
        
        // FFT

        struct ChannelState
        {
            std::vector<float> inputBuffer;
            std::vector<float> outputBuffer;
            int writePos = 0;
            bool initialised = false;
        };

        static constexpr float HARMONIC_DETECTION_THRESHOLD = 0.03f; // 0.03f
        
        double _sampleRate;
        
        // playback, tempo and time signature

        bool isPlaying = false;
        int timeSigNumerator = 4;
        int timeSigDenominator = 4;
        double tempo = 120.0;
        
        // parameters

        std::atomic<float>* linkEnabled;
        std::atomic<float>* splitFreq;
        std::atomic<Parameters::SplitMode> splitMode;
        std::atomic<float>* dryWetMix;
        std::atomic<Parameters::DistortionType> loDistType;
        std::atomic<float>* loDistInputLevel;
        std::atomic<float>* loDistDrive;
        std::atomic<float>* loDistParam;
        std::atomic<Parameters::DistortionType> hiDistType;
        std::atomic<float>* hiDistInputLevel;
        std::atomic<float>* hiDistDrive;
        std::atomic<float>* hiDistParam;
        
        inline void applyDistortion(
            float* lowChannelData, float* highChannelData, unsigned long lowChannelSize, unsigned long highChannelSize
        ) {
            bool jointProcessing = ParameterUtilities::floatToBool( *linkEnabled );

            switch ( loDistType )
            {
                case Parameters::DistortionType::Off:
                    break;
                
                case Parameters::DistortionType::BitCrusher:
                    lowBitCrusher.apply( lowChannelData, lowChannelSize );
                    if ( jointProcessing ) {
                        lowBitCrusher.apply( highChannelData, highChannelSize );
                    }
                    break;
                
                case Parameters::DistortionType::Fuzz:
                    lowFuzz.apply( lowChannelData, lowChannelSize );
                    if ( jointProcessing ) {
                        lowFuzz.apply( highChannelData, highChannelSize );
                    }
                    break;

                case Parameters::DistortionType::WaveFolder:
                    lowWaveFolder.apply( lowChannelData, lowChannelSize );
                    if ( jointProcessing ) {
                        lowWaveFolder.apply( highChannelData, highChannelSize );
                    }
                    break;

                case Parameters::DistortionType::WaveShaper:
                    lowWaveShaper.apply( lowChannelData, lowChannelSize );
                    if ( jointProcessing ) {
                        lowWaveShaper.apply( highChannelData, highChannelSize );
                    }
                    break;
            }

            if ( jointProcessing ) {
                return; // hi channel processed above
            }

            switch ( hiDistType )
            {
                case Parameters::DistortionType::Off:
                    break;

                case Parameters::DistortionType::BitCrusher:
                    hiBitCrusher.apply( highChannelData, highChannelSize );
                    break;

                case Parameters::DistortionType::Fuzz:
                    hiFuzz.apply( highChannelData, highChannelSize );
                    break;

                case Parameters::DistortionType::WaveFolder:
                    hiWaveFolder.apply( highChannelData, highChannelSize );
                    break;

                case Parameters::DistortionType::WaveShaper:
                    hiWaveShaper.apply( highChannelData, highChannelSize );
                    break;
            }
        }
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioPluginAudioProcessor )
};
