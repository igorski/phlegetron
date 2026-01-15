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
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "utils/MathUtilities.h"

AudioPluginAudioProcessor::AudioPluginAudioProcessor(): AudioProcessor( BusesProperties()
    #if ! JucePlugin_IsMidiEffect
        #if ! JucePlugin_IsSynth
        .withInput( "Input",  juce::AudioChannelSet::stereo(), true )
        #endif
        .withOutput( "Output", juce::AudioChannelSet::stereo(), true )
    #endif
    ),
    ParameterSubscriber(),
    parameters( *this, nullptr, "PARAMETERS", createParameterLayout()),
    parameterListener( *this, parameters ) 
{
    // grab a reference to all automatable parameters and initialize the values (to their defined defaults)

    linkEnabled      = parameters.getRawParameterValue( Parameters::LINK_ENABLED );
    splitFreq        = parameters.getRawParameterValue( Parameters::SPLIT_FREQ );
    splitMode        = static_cast<Parameters::SplitMode>( parameters.getRawParameterValue( Parameters::SPLIT_MODE )->load());
    dryWetMix        = parameters.getRawParameterValue( Parameters::DRY_WET_MIX );
    loDistType       = static_cast<Parameters::DistortionType>( parameters.getRawParameterValue( Parameters::LO_DIST_TYPE )->load());
    loDistInputLevel = parameters.getRawParameterValue( Parameters::LO_DIST_INPUT );
    loDistDrive      = parameters.getRawParameterValue( Parameters::LO_DIST_DRIVE );
    loDistParam      = parameters.getRawParameterValue( Parameters::LO_DIST_PARAM );
    hiDistType       = static_cast<Parameters::DistortionType>( parameters.getRawParameterValue( Parameters::HI_DIST_TYPE )->load());
    hiDistInputLevel = parameters.getRawParameterValue( Parameters::HI_DIST_INPUT );
    hiDistDrive      = parameters.getRawParameterValue( Parameters::HI_DIST_DRIVE );
    hiDistParam      = parameters.getRawParameterValue( Parameters::HI_DIST_PARAM );
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    // nowt...
}

/* configuration */

const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported( const BusesLayout& layouts ) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused( layouts );
    return true;
  #else
    if ( layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
         layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) {
        return false;
    }

   #if ! JucePlugin_IsSynth
    if ( layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet()) {
        return false;
    }
   #endif

    return true;
  #endif
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

/* programs */

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram( int index )
{
    juce::ignoreUnused( index );
}

const juce::String AudioPluginAudioProcessor::getProgramName( int index )
{
    juce::ignoreUnused( index );
    return {};
}

void AudioPluginAudioProcessor::changeProgramName( int index, const juce::String& newName )
{
    juce::ignoreUnused( index, newName );
}

/* automatable parameters */

void AudioPluginAudioProcessor::updateParameters()
{
    splitMode = static_cast<Parameters::SplitMode>(
        parameters.getRawParameterValue( Parameters::SPLIT_MODE )->load()
    );
    loDistType = static_cast<Parameters::DistortionType>(
        parameters.getRawParameterValue( Parameters::LO_DIST_TYPE )->load()
    );
    hiDistType = static_cast<Parameters::DistortionType>(
        parameters.getRawParameterValue( Parameters::HI_DIST_TYPE )->load()
    );

    switch ( loDistType )
    {
        case Parameters::DistortionType::Off:
            break;

        case Parameters::DistortionType::BitCrusher:
            lowBitCrusher.setLevel( *loDistInputLevel );
            lowBitCrusher.setDownsampling( *loDistDrive );
            lowBitCrusher.setAmount( *loDistParam );
            break;

        case Parameters::DistortionType::Fuzz:
            lowFuzz.setInputLevel( *loDistInputLevel );
            lowFuzz.setThreshold( *loDistDrive );
            lowFuzz.setCutOff( *loDistParam );
            break;

        case Parameters::DistortionType::WaveFolder:
            lowWaveFolder.setLevel( *loDistInputLevel );
            lowWaveFolder.setDrive( *loDistDrive );
            lowWaveFolder.setThreshold( *loDistDrive ); // make positive threshold equal to drive level
            lowWaveFolder.setThresholdNegative( *loDistParam );
            break;

        case Parameters::DistortionType::WaveShaper:
            lowWaveShaper.setOutputLevel( *loDistInputLevel );
            lowWaveShaper.setAmount( *loDistDrive );
            lowWaveShaper.setShape( *loDistParam );
            break;
    }

    switch ( hiDistType )
    {
        case Parameters::DistortionType::Off:
            break;
            
        case Parameters::DistortionType::BitCrusher:
            hiBitCrusher.setLevel( *hiDistInputLevel );
            hiBitCrusher.setDownsampling( *hiDistDrive );
            hiBitCrusher.setAmount( *hiDistParam );
            break;

        case Parameters::DistortionType::Fuzz:
            hiFuzz.setInputLevel( *hiDistInputLevel );
            hiFuzz.setThreshold( *hiDistDrive );
            hiFuzz.setCutOff( *hiDistParam );
            break;

        case Parameters::DistortionType::WaveFolder:
            hiWaveFolder.setLevel( *hiDistInputLevel );
            hiWaveFolder.setDrive( *hiDistDrive );
            hiWaveFolder.setThreshold( *hiDistDrive ); // make positive threshold equal to drive level
            hiWaveFolder.setThresholdNegative( *hiDistParam );
            break;

        case Parameters::DistortionType::WaveShaper:
            hiWaveShaper.setOutputLevel( *hiDistInputLevel );
            hiWaveShaper.setAmount( *hiDistDrive );
            hiWaveShaper.setShape( *hiDistParam );
            break;
    }

    for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel ) {
        prepareCrossoverFilter( lowPass[ channel ],  juce::dsp::LinkwitzRileyFilterType::lowpass,  *splitFreq );
        prepareCrossoverFilter( highPass[ channel ], juce::dsp::LinkwitzRileyFilterType::highpass, *splitFreq );
    }
}

/* resource management */

void AudioPluginAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    juce::ignoreUnused( samplesPerBlock );
    
    _sampleRate = sampleRate;

    juce::dsp::ProcessSpec spec {
        sampleRate,
        ( juce::uint32 ) samplesPerBlock,
        ( juce::uint32 ) 1 // one filter per channel
    };

    for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel ) {
        lowPass[ channel ].prepare( spec );
        highPass[ channel ].prepare( spec );

        prepareCrossoverFilter( lowPass[ channel ],  juce::dsp::LinkwitzRileyFilterType::lowpass,  Parameters::Config::SPLIT_FREQ_DEF );
        prepareCrossoverFilter( highPass[ channel ], juce::dsp::LinkwitzRileyFilterType::highpass, Parameters::Config::SPLIT_FREQ_DEF );
    }
    lowBuffer.resize(( size_t ) samplesPerBlock );
    highBuffer.resize(( size_t ) samplesPerBlock );

    // dispose previously allocated resources
    releaseResources();
    
    // align values with model
    updateParameters();
}

void AudioPluginAudioProcessor::releaseResources()
{
    // nowt...
}

/* rendering */

void AudioPluginAudioProcessor::processBlock( juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages )
{
    juce::ignoreUnused( midiMessages );
    juce::ScopedNoDenormals noDenormals;
  
    int channelAmount = buffer.getNumChannels();
    int bufferSize = buffer.getNumSamples();
    auto uBufferSize = static_cast<unsigned long>( bufferSize );
    
    float dryMix  = 1.f - *dryWetMix;
    float wetMix  = *dryWetMix;
    bool blendDry = dryMix > 0.f;

    const int fftOrder    = 11; // 2048-point FFT
    const int fftSize     = 1 << fftOrder;
    constexpr int hopSize = fftSize / 2;
    const float baseFreq  = *splitFreq;

    static juce::dsp::FFT fft( fftOrder );

    // these are static as a cheap way to avoid allocation overhead

    static std::vector<float> window( fftSize );
    static bool windowInit = false;
    if ( !windowInit )
    {
        for ( size_t n = 0; n < fftSize; ++n ) {
            window[ n ] = 0.5f - 0.5f * std::cos( 2.f * juce::MathConstants<float>::pi * n / fftSize );
        }
        windowInit = true;
    }

    // per channel processing

    for ( int channel = 0; channel < channelAmount; ++channel )
    {
        if ( buffer.getReadPointer( channel ) == nullptr ) {
            continue;
        }
        
        // process mode 1: EQ based split

        if ( splitMode == Parameters::SplitMode::EQ ) {

            auto channelBuffer = buffer.getReadPointer( channel );
            auto low  = lowBuffer.data();
            auto high = highBuffer.data();
            
            std::memcpy( low,  channelBuffer, sizeof( float ) * uBufferSize );
            std::memcpy( high, channelBuffer, sizeof( float ) * uBufferSize );

            // apply Linkwitz–Riley filtering for a clean crossover separation

            juce::dsp::AudioBlock<float> lowBlock( &low,  1, uBufferSize );
            juce::dsp::AudioBlock<float> highBlock( &high, 1, uBufferSize );

            lowPass[ channel ].process( juce::dsp::ProcessContextReplacing<float>( lowBlock ));
            highPass[ channel ].process( juce::dsp::ProcessContextReplacing<float>( highBlock ));

            applyDistortion( low, high, uBufferSize, uBufferSize );

            // write the effected buffer into the output
    
            for ( int i = 0; i < bufferSize; ++i ) {
                auto dry = buffer.getSample( channel, i ) * dryMix;
                auto wet = ( ATTENUATION_FACTOR * ( low[ i ] + high [ i ])) * wetMix;

                buffer.setSample( channel, i, MathUtilities::clamp( dry + wet ));
            }
        }
        else {
            // process mode 2: harmonic bin splitting

            auto* channelData = buffer.getWritePointer( channel );
            std::vector<float> inBuffer( uBufferSize );

            if ( blendDry ) {
                // copy the dry signal at its blend value
                for ( size_t i = 0; i < uBufferSize; ++i ) {
                    inBuffer[ i ] = channelData[ i ] * dryMix;
                }
            }
            
            static std::array<ChannelState, 2> channelStates;

            auto& channelState = channelStates[ static_cast<unsigned long>( channel )];
            if ( !channelState.initialised ) {
                channelState.inputBuffer.assign ( fftSize, 0.0f );
                channelState.outputBuffer.assign( fftSize, 0.0f );
                channelState.initialised = true;
            }

            int samplesProcessed = 0;
            while ( samplesProcessed < bufferSize )
            {
                // Write new input into circular inputBuffer
                const int samplesToCopy = std::min( hopSize, bufferSize - samplesProcessed );
                std::memmove(
                    channelState.inputBuffer.data(), channelState.inputBuffer.data() + hopSize, ( fftSize - hopSize ) * sizeof( float )
                );
                std::memcpy(
                    channelState.inputBuffer.data() + ( fftSize - hopSize ), channelData + samplesProcessed, static_cast<unsigned long>( samplesToCopy ) * sizeof( float )
                );

                // apply FFT

                std::vector<float> fftTime( fftSize * 2, 0.0f );
                for ( size_t i = 0; i < fftSize; ++i ) {
                    fftTime[ i ] = channelState.inputBuffer[ i ] * window[ i ];
                }
                fft.performRealOnlyForwardTransform( fftTime.data());

                // convert to complex
                
                std::vector<std::complex<float>> spec( fftSize / 2 );
                for ( size_t i = 0; i < fftSize / 2; ++i ) {
                    spec[ i ] = { fftTime[ 2 * i ], fftTime[ 2 * i + 1 ]};
                }

                // split spectrum by harmonic proximity
                
                std::vector<std::complex<float>> specA( fftSize / 2 ), specB( fftSize / 2 );
                for ( size_t bin = 0; bin < fftSize / 2; ++bin )
                {
                    float freq = bin * ( float ) _sampleRate / fftSize;
                    bool harmonic = false;
                    for ( int n = 1; n <= 12; ++n ) {
                        float h = n * baseFreq;
                        // @todo modulate 0.03f ? to 0.3 is nicely wicked (at lower freqs)
                        if ( std::abs( freq - h ) < h * 0.03f ) {
                            harmonic = true;
                            break;
                        }
                    }
                    if ( harmonic ) {
                        specA[ bin ] = spec[ bin ];
                    } else {
                        specB[ bin ] = spec[ bin ];
                    }
                }

                auto iFFT = [&]( const std::vector<std::complex<float>>& src, std::vector<float>& dst )
                {
                    for ( size_t i = 0; i < fftSize / 2; ++i ) {
                        dst[ 2 * i ]     = src[ i ].real();
                        dst[ 2 * i + 1 ] = src[ i ].imag();
                    }
                    fft.performRealOnlyInverseTransform( dst.data() );
                };

                // inverse FFT and OLA accumulate

                std::vector<float> tempA( fftSize * 2, 0.0f ), tempB( fftSize * 2, 0.0f );
                iFFT( specA, tempA );
                iFFT( specB, tempB );

                applyDistortion( tempA.data(), tempB.data(), tempA.size(), tempB.size() );

                // sum and window

                for ( size_t i = 0; i < fftSize; ++i ) {
                    channelState.outputBuffer[ i ] += ( ATTENUATION_FACTOR * ( tempA[ i ] + tempB[ i ])) * window[ i ];
                }

                // write samples to output (for the hopSize)

                for ( int i = 0; i < hopSize && ( samplesProcessed + i < bufferSize ); ++i ) {
                    channelData[ samplesProcessed + i ] = channelState.outputBuffer[ static_cast<unsigned long>( i )] * wetMix;
                }

                // shift output buffer for next overlap

                std::memmove(
                    channelState.outputBuffer.data(), channelState.outputBuffer.data() + hopSize, ( fftSize - hopSize ) * sizeof( float )
                );
                std::fill( channelState.outputBuffer.begin() + ( fftSize - hopSize ), channelState.outputBuffer.end(), 0.0f );

                samplesProcessed += hopSize;
            }

            if ( blendDry ) {
                for ( size_t i = 0; i < uBufferSize; ++i ) {
                    channelData[ i ] += inBuffer[ i ];
                }
            }
        }
    }
}

/* editor */

bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor( *this, parameters );
}

/* persistence */

void AudioPluginAudioProcessor::getStateInformation( juce::MemoryBlock& destData )
{
    juce::MemoryOutputStream stream( destData, true );
    parameters.state.writeToStream( stream );
}

void AudioPluginAudioProcessor::setStateInformation( const void* data, int sizeInBytes )
{
    juce::ValueTree tree = juce::ValueTree::readFromData( data, static_cast<unsigned long>( sizeInBytes ));
    if ( tree.isValid()) {
        parameters.state = tree;
    }
}

/* runtime state */

bool AudioPluginAudioProcessor::alignWithSequencer( juce::Optional<juce::AudioPlayHead::PositionInfo> positionInfo )
{
    bool wasPlaying = isPlaying;
    isPlaying = positionInfo->getIsPlaying();

    if ( !wasPlaying && isPlaying ) {
        // sequencer started playback
        int channelAmount = getTotalNumOutputChannels();

        for ( int channel = 0; channel < channelAmount; ++channel ) {
            // nowt   
        }
    }

    bool hasChange = false;

    auto curTempo = positionInfo->getBpm();
    auto timeSig  = positionInfo->getTimeSignature();

    if ( curTempo.hasValue() && !juce::approximatelyEqual( tempo, *curTempo )) {
        tempo = *curTempo;
        hasChange = true;
    }

    if ( timeSig.hasValue() && ( timeSigNumerator != timeSig->numerator || timeSigDenominator != timeSig->denominator )) {
        timeSigNumerator   = timeSig->numerator;
        timeSigDenominator = timeSig->denominator;

        hasChange = true;
    }
    return hasChange;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}