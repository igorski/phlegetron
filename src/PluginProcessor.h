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
#include "modules/bitcrusher/Bitcrusher.h"
#include "modules/dcfilter/DCFilter.h"
#include "modules/fft/FFT.h"
#include "modules/fuzz/Fuzz.h"
#include "modules/gain/AutoMakeUpGain.h"
#include "modules/smoother/Smoother.h"
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
        void applyParameters( int bufferSize );

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
        
        juce::dsp::LinkwitzRileyFilter<float> loPass[ MAX_CHANNELS ];
        juce::dsp::LinkwitzRileyFilter<float> hiPass[ MAX_CHANNELS ];
        AutoMakeUpGain loMakeup[ MAX_CHANNELS ];
        AutoMakeUpGain hiMakeup[ MAX_CHANNELS ];
        DCFilter dcFilters[ MAX_CHANNELS ];
        std::vector<float> loBuffer;
        std::vector<float> hiBuffer;

        // parameter smoothing (prevent glitches)
        
        static constexpr float PARAM_RAMP_TIME = 0.2f;
        
        Smoother splitFreqSmoothed;
        Smoother loLevelSmoothed;
        Smoother loDriveSmoothed;
        Smoother loParamSmoothed;
        Smoother hiLevelSmoothed;
        Smoother hiDriveSmoothed;
        Smoother hiParamSmoothed;
        
        inline void prepareCrossoverFilter(
            juce::dsp::LinkwitzRileyFilter<float> &filter, juce::dsp::LinkwitzRileyFilterType type, float frequency
        ) {
            filter.reset();
            filter.setCutoffFrequency( frequency );
            filter.setType( type );
        }

        // distortion modules, most are stateless w/regards to past inputs and can
        // be reused across channels, with exception of BitCrusher

        BitCrusher loBitCrusher[ MAX_CHANNELS ];
        BitCrusher hiBitCrusher[ MAX_CHANNELS ];
        Fuzz loFuzz;
        Fuzz hiFuzz;
        WaveFolder loWaveFolder;
        WaveFolder hiWaveFolder;
        WaveShaper loWaveShaper;
        WaveShaper hiWaveShaper;

        // read / write buffers

        std::vector<float> loPre;
        std::vector<float> hiPre;
        std::vector<float> inBuffer;
        std::vector<float> specA;
        std::vector<float> specB;
        
        // FFT processing

        struct ChannelState
        {
            std::vector<float> inputBuffer;
            std::vector<float> outputBuffer;
            int writePos = 0;
            bool initialised = false;
        };
        std::array<ChannelState, MAX_CHANNELS> channelStates;
        FFT fft;
        
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
            const int channel, float* loChannelData, float* hiChannelData,
            const unsigned long loChannelSize, const unsigned long hiChannelSize
        ) {
            // distortions aren't stateful, when jointProcessing is true, we apply
            // the settings of the low distortion onto the high channel

            bool jointProcessing = ParameterUtilities::floatToBool( *linkEnabled );

            switch ( loDistType )
            {
                case Parameters::DistortionType::Off:
                    break;
                
                case Parameters::DistortionType::BitCrusher:
                    loBitCrusher[ channel ].apply( loChannelData, loChannelSize );
                    if ( jointProcessing ) {
                        loBitCrusher[ channel ].apply( hiChannelData, hiChannelSize );
                    }
                    break;
                
                case Parameters::DistortionType::Fuzz:
                    loFuzz.apply( loChannelData, loChannelSize );
                    if ( jointProcessing ) {
                        loFuzz.apply( hiChannelData, hiChannelSize );
                    }
                    break;

                case Parameters::DistortionType::WaveFolder:
                    loWaveFolder.apply( loChannelData, loChannelSize );
                    if ( jointProcessing ) {
                        loWaveFolder.apply( hiChannelData, hiChannelSize );
                    }
                    break;

                case Parameters::DistortionType::WaveShaper:
                    loWaveShaper.apply( loChannelData, loChannelSize );
                    if ( jointProcessing ) {
                        loWaveShaper.apply( hiChannelData, hiChannelSize );
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
                    hiBitCrusher[ channel ].apply( hiChannelData, hiChannelSize );
                    break;

                case Parameters::DistortionType::Fuzz:
                    hiFuzz.apply( hiChannelData, hiChannelSize );
                    break;

                case Parameters::DistortionType::WaveFolder:
                    hiWaveFolder.apply( hiChannelData, hiChannelSize );
                    break;

                case Parameters::DistortionType::WaveShaper:
                    hiWaveShaper.apply( hiChannelData, hiChannelSize );
                    break;
            }
        }
        
        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR( AudioPluginAudioProcessor )
};
