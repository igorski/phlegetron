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

    splitFreqSmoothed.set( *splitFreq );
    loLevelSmoothed.set( *loDistInputLevel );
    loDriveSmoothed.set( *loDistDrive );
    loParamSmoothed.set( *loDistParam );
    hiLevelSmoothed.set( *hiDistInputLevel );
    hiDriveSmoothed.set( *hiDistDrive );
    hiParamSmoothed.set( *hiDistParam );
}

void AudioPluginAudioProcessor::applyParameters( int bufferSize )
{
    if ( !splitFreqSmoothed.isDone() ) {
        const float baseFreq = splitFreqSmoothed.peek( bufferSize );

        for ( int channel = 0; channel < MAX_CHANNELS; ++channel ) {
            loPass[ channel ].setCutoffFrequency( baseFreq );
            hiPass[ channel ].setCutoffFrequency( baseFreq );
        }
    }
    bool updateLoDistortion = !loLevelSmoothed.isDone() || !loDriveSmoothed.isDone() || !loParamSmoothed.isDone();
    bool updateHiDistortion = !hiLevelSmoothed.isDone() || !hiDriveSmoothed.isDone() || !hiParamSmoothed.isDone();

    if ( updateLoDistortion )
    {
        const float level = loLevelSmoothed.peek( bufferSize );
        const float drive = loDriveSmoothed.peek( bufferSize );
        const float param = loParamSmoothed.peek( bufferSize );

        switch ( loDistType )
        {
            case Parameters::DistortionType::Off:
                break;

            case Parameters::DistortionType::BitCrusher:
                for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel ) {
                    loBitCrusher[ channel ].setLevel( level );
                    loBitCrusher[ channel ].setDownsampling( drive );
                    loBitCrusher[ channel ].setAmount( param );
                }
                break;

            case Parameters::DistortionType::Fuzz:
                loFuzz.setInputLevel( level );
                loFuzz.setThreshold( drive );
                loFuzz.setCutOff( param );
                break;

            case Parameters::DistortionType::WaveFolder:
                loWaveFolder.setLevel( level );
                loWaveFolder.setDrive( drive );
                loWaveFolder.setThreshold( param );
                // loWaveFolder.setThresholdNegative( param );
                break;

            case Parameters::DistortionType::WaveShaper:
                loWaveShaper.setOutputLevel( level );
                loWaveShaper.setAmount( drive );
                loWaveShaper.setShape( param );
                break;
        }
    }

    if ( updateHiDistortion )
    {
        const float level = hiLevelSmoothed.peek( bufferSize );
        const float drive = hiDriveSmoothed.peek( bufferSize );
        const float param = hiParamSmoothed.peek( bufferSize );

        switch ( hiDistType )
        {
            case Parameters::DistortionType::Off:
                break;
                
            case Parameters::DistortionType::BitCrusher:
                for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel ) {
                    hiBitCrusher[ channel ].setLevel( level );
                    hiBitCrusher[ channel ].setDownsampling( drive );
                    hiBitCrusher[ channel ].setAmount( param );
                }
                break;

            case Parameters::DistortionType::Fuzz:
                hiFuzz.setInputLevel( level );
                hiFuzz.setThreshold( drive );
                hiFuzz.setCutOff( param );
                break;

            case Parameters::DistortionType::WaveFolder:
                hiWaveFolder.setLevel( level );
                hiWaveFolder.setDrive( drive );
                hiWaveFolder.setThreshold( param );
                // hiWaveFolder.setThresholdNegative( param );
                break;

            case Parameters::DistortionType::WaveShaper:
                hiWaveShaper.setOutputLevel( level );
                hiWaveShaper.setAmount( drive );
                hiWaveShaper.setShape( param );
                break;
        }
    }
}

/* resource management */

void AudioPluginAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    fft.update( sampleRate );

    splitFreqSmoothed.init( sampleRate, PARAM_RAMP_TIME, *splitFreq );
    loLevelSmoothed.init( sampleRate, PARAM_RAMP_TIME, *loDistInputLevel );
    loDriveSmoothed.init( sampleRate, PARAM_RAMP_TIME, *loDistDrive );
    loParamSmoothed.init( sampleRate, PARAM_RAMP_TIME, *loDistParam );
    hiLevelSmoothed.init( sampleRate, PARAM_RAMP_TIME, *hiDistInputLevel );
    hiDriveSmoothed.init( sampleRate, PARAM_RAMP_TIME, *hiDistDrive );
    hiParamSmoothed.init( sampleRate, PARAM_RAMP_TIME, *hiDistParam );

    juce::dsp::ProcessSpec spec {
        sampleRate,
        ( juce::uint32 ) samplesPerBlock,
        ( juce::uint32 ) 1 // one filter per channel
    };

    for ( size_t channel = 0; channel < MAX_CHANNELS; ++channel )
    {
        loMakeup[ channel ].prepare( sampleRate );
        hiMakeup[ channel ].prepare( sampleRate );

        dcFilters[ channel ].init( sampleRate );
        
        loPass[ channel ].prepare( spec );
        hiPass[ channel ].prepare( spec );

        prepareCrossoverFilter( loPass[ channel ], juce::dsp::LinkwitzRileyFilterType::lowpass,  splitFreqSmoothed.get() );
        prepareCrossoverFilter( hiPass[ channel ], juce::dsp::LinkwitzRileyFilterType::highpass, splitFreqSmoothed.get() );

        auto& channelState = channelStates[ channel ];
        if ( !channelState.initialised ) {
            channelState.inputBuffer.assign ( Parameters::FFT::SIZE, 0.0f );
            channelState.outputBuffer.assign( Parameters::FFT::SIZE, 0.0f );
            channelState.initialised = true;
        }
    }
    loBuffer.resize(( size_t ) samplesPerBlock );
    hiBuffer.resize(( size_t ) samplesPerBlock );

    loPre.resize(( size_t ) samplesPerBlock );
    hiPre.resize(( size_t ) samplesPerBlock );
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
    bool needsFiltering = loDistType == Parameters::DistortionType::WaveFolder || ( !ParameterUtilities::floatToBool( *linkEnabled ) && hiDistType == Parameters::DistortionType::WaveFolder );
    
    // update module properties with smoothed changes to prevent crackling

    applyParameters( bufferSize );

    // per channel processing

    for ( int channel = 0; channel < channelAmount; ++channel )
    {
        if ( buffer.getReadPointer( channel ) == nullptr ) {
            continue;
        }

        int channelNum = channel % MAX_CHANNELS; // keep in range of modules allocated to MAX_CHANNELS
        auto* channelData = buffer.getWritePointer( channel );
            
        // process mode 1: EQ based split

        if ( splitMode == Parameters::SplitMode::EQ ) {

            auto channelBuffer = buffer.getReadPointer( channel );
            auto lo = loBuffer.data();
            auto hi = hiBuffer.data();
            
            std::memcpy( lo, channelBuffer, sizeof( float ) * uBufferSize );
            std::memcpy( hi, channelBuffer, sizeof( float ) * uBufferSize );

            // apply Linkwitz–Riley filtering for a clean crossover separation

            juce::dsp::AudioBlock<float> loBlock( &lo, 1, uBufferSize );
            juce::dsp::AudioBlock<float> hiBlock( &hi, 1, uBufferSize );

            loPass[ channelNum ].process( juce::dsp::ProcessContextReplacing<float>( loBlock ));
            hiPass[ channelNum ].process( juce::dsp::ProcessContextReplacing<float>( hiBlock ));

            // save the pre-distorted state of the filtered buffer...

            std::memcpy( loPre.data(), lo, sizeof( float ) * uBufferSize );
            std::memcpy( hiPre.data(), hi, sizeof( float ) * uBufferSize );

            // ...distort

            applyDistortion( channel, lo, hi, uBufferSize, uBufferSize );

            // ...and apply make-up gain to keep large volume jumps in check

            loMakeup[ channelNum ].apply( loPre.data(), lo, bufferSize );
            hiMakeup[ channelNum ].apply( hiPre.data(), hi, bufferSize );

            // write the effected buffer into the output
    
            for ( int i = 0; i < bufferSize; ++i ) {
                auto dry = channelData[ i ] * dryMix;
                auto wet = ( lo[ i ] + hi[ i ]) * wetMix;

                channelData[ i ] = MathUtilities::clamp( dry + wet );
            }
        }
        else {
            // process mode 2: harmonic bin splitting

            auto& channelState = channelStates[ ( size_t ) channelNum ];

            std::memcpy( inBuffer.data(), channelData, sizeof( float ) * uBufferSize );

            // note we use splitFreq (the target of splitFreqSmoothed) instead of the current
            // smoothed value to prevent calculation overhead, this (non-interpolated) value is safe for masking

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

                applyDistortion( channel, specA.data(), specB.data(), specA.size(), specB.size() );

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

            loMakeup[ channelNum ].apply( inBuffer.data(), channelData, bufferSize );

            if ( blendDry ) {
                for ( size_t i = 0; i < uBufferSize; ++i ) {
                    channelData[ i ] += ( inBuffer[ i ] * dryMix );
                }
            }
        }
        // certain processes can benefit from removing ultra- and infrasonic noise from the signal
        if ( needsFiltering ) {
            dcFilters[ channelNum ].apply( channelData, uBufferSize );
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