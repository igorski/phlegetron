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
#include "editor/PluginEditor.h"
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

    // prepare resources for FFT processing

    specA.resize(( size_t ) Parameters::FFT::DOUBLE_SIZE, 0.f );
    specB.resize(( size_t ) Parameters::FFT::DOUBLE_SIZE, 0.f );
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
            lowWaveFolder.setThreshold( *loDistParam );
            // lowWaveFolder.setThresholdNegative( *loDistParam );
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
            hiWaveFolder.setThreshold( *loDistParam );
            // hiWaveFolder.setThresholdNegative( *hiDistParam );
            break;

        case Parameters::DistortionType::WaveShaper:
            hiWaveShaper.setOutputLevel( *hiDistInputLevel );
            hiWaveShaper.setAmount( *hiDistDrive );
            hiWaveShaper.setShape( *hiDistParam );
            break;
    }
    splitFreqSmoothed.setTargetValue( *splitFreq );
}

/* resource management */

void AudioPluginAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    juce::ignoreUnused( samplesPerBlock );

    fft.update( sampleRate );
    
    splitFreqSmoothed.reset( sampleRate, 0.02 );
    splitFreqSmoothed.setCurrentAndTargetValue( *splitFreq );

    juce::dsp::ProcessSpec spec {
        sampleRate,
        ( juce::uint32 ) samplesPerBlock,
        ( juce::uint32 ) 1 // one filter per channel
    };

    for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel ) {
        lowMakeup[ channel ].prepare( sampleRate );
        highMakeup[ channel ].prepare( sampleRate );

        lowPass[ channel ].prepare( spec );
        highPass[ channel ].prepare( spec );

        prepareCrossoverFilter( lowPass[ channel ],  juce::dsp::LinkwitzRileyFilterType::lowpass,  splitFreqSmoothed.getCurrentValue() );
        prepareCrossoverFilter( highPass[ channel ], juce::dsp::LinkwitzRileyFilterType::highpass, splitFreqSmoothed.getCurrentValue() );

        auto& channelState = channelStates[ channel ];
        if ( !channelState.initialised ) {
            channelState.inputBuffer.assign ( Parameters::FFT::SIZE, 0.0f );
            channelState.outputBuffer.assign( Parameters::FFT::SIZE, 0.0f );
            channelState.initialised = true;
        }
    }
    lowBuffer.resize(( size_t ) samplesPerBlock );
    highBuffer.resize(( size_t ) samplesPerBlock );

    lowPre.resize(( size_t ) samplesPerBlock );
    highPre.resize(( size_t ) samplesPerBlock );
    inBuffer.resize(( size_t ) samplesPerBlock );

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
    
    // prepare gain staging

    float dryMix  = 1.f - *dryWetMix;
    float wetMix  = *dryWetMix;
    bool blendDry = dryMix > 0.f;

    // update filters with smoothed frequency changes to prevent crackling

    const float baseFreq = splitFreqSmoothed.getNextValue();
    splitFreqSmoothed.skip( bufferSize );

    for ( int channel = 0; channel < MAX_CHANNELS; ++channel ) {
        lowPass[ channel ].setCutoffFrequency( baseFreq );
        highPass[ channel ].setCutoffFrequency( baseFreq );
    }
    
    // per channel processing

    for ( int channel = 0; channel < channelAmount; ++channel )
    {
        if ( buffer.getReadPointer( channel ) == nullptr ) {
            continue;
        }

        int channelNum = channel % MAX_CHANNELS; // keep in range of modules allocated to MAX_CHANNELS

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

            lowPass[ channelNum ].process( juce::dsp::ProcessContextReplacing<float>( lowBlock ));
            highPass[ channelNum ].process( juce::dsp::ProcessContextReplacing<float>( highBlock ));

            // save the pre-distorted state of the filtered buffer...

            std::memcpy( lowPre.data(),  low,  sizeof( float ) * uBufferSize );
            std::memcpy( highPre.data(), high, sizeof( float ) * uBufferSize );

            // ...distort

            applyDistortion( low, high, uBufferSize, uBufferSize );

            // ...and apply make-up gain to keep large volume jumps in check

            lowMakeup[ channelNum ].apply( lowPre.data(), low, bufferSize );
            highMakeup[ channelNum ].apply( highPre.data(), high, bufferSize );

            // write the effected buffer into the output
    
            for ( int i = 0; i < bufferSize; ++i ) {
                auto dry = buffer.getSample( channel, i ) * dryMix;
                auto wet = ( low[ i ] + high [ i ]) * wetMix;

                buffer.setSample( channel, i, MathUtilities::clamp( dry + wet ));
            }
        }
        else {
            // process mode 2: harmonic bin splitting

            auto* channelData  = buffer.getWritePointer( channel );
            auto& channelState = channelStates[ ( size_t ) channelNum ];

            std::memcpy( inBuffer.data(), channelData, sizeof( float ) * uBufferSize );

            // note we use splitFreq (the target of splitFreqSmoothed) instead of the current
            // smoothed value to prevent calculation overhead, this value is safe for masking

            fft.calculateHarmonics( splitFreq->load() );
            
            unsigned long samplesProcessed = 0;
            while ( samplesProcessed < uBufferSize )
            {
                // Write new input into circular inputBuffer
                const unsigned long samplesToCopy = std::min( Parameters::FFT::HOP_SIZE, uBufferSize - samplesProcessed );
                std::memmove(
                    channelState.inputBuffer.data(), channelState.inputBuffer.data() + Parameters::FFT::HOP_SIZE, ( Parameters::FFT::SIZE - Parameters::FFT::HOP_SIZE ) * sizeof( float )
                );
                std::memcpy(
                    channelState.inputBuffer.data() + ( Parameters::FFT::SIZE - Parameters::FFT::HOP_SIZE ), channelData + samplesProcessed, static_cast<unsigned long>( samplesToCopy ) * sizeof( float )
                );

                // apply FFT to split input signal into specA and specB by harmonic bins

                fft.split( channelState.inputBuffer, specA, specB );

                // distort

                applyDistortion( specA.data(), specB.data(), specA.size(), specB.size() );

                // sum and apply window (windowing ensures overlap-add works correctly)

                fft.sum( channelState.outputBuffer, specA, specB );

                // write samples to output

                for ( size_t i = 0; i < Parameters::FFT::HOP_SIZE && ( samplesProcessed + i < uBufferSize ); ++i ) {
                    channelData[ samplesProcessed + i ] = channelState.outputBuffer[ static_cast<unsigned long>( i )] * wetMix;
                }

                // shift output buffer for next overlap

                std::memmove(
                    channelState.outputBuffer.data(), channelState.outputBuffer.data() + Parameters::FFT::HOP_SIZE,
                    ( Parameters::FFT::SIZE - Parameters::FFT::HOP_SIZE ) * sizeof( float )
                );
                std::fill(
                    channelState.outputBuffer.begin() + static_cast<long>( Parameters::FFT::SIZE - Parameters::FFT::HOP_SIZE ),
                    channelState.outputBuffer.end(), 0.0f
                );
                samplesProcessed += Parameters::FFT::HOP_SIZE;
            }

            // apply make-up gain to keep large volume jumps in check

            lowMakeup[ channelNum ].apply( inBuffer.data(), channelData, bufferSize );

            if ( blendDry ) {
                for ( size_t i = 0; i < uBufferSize; ++i ) {
                    channelData[ i ] += ( inBuffer[ i ] * dryMix );
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