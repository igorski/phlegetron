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
#include "PluginProcessor.h"
#include "PluginEditor.h"

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

    dryWetMix           = parameters.getRawParameterValue( Parameters::DRY_WET_MIX );
    distInputLevel      = parameters.getRawParameterValue( Parameters::DIST_INPUT );
    distCutoffThreshold = parameters.getRawParameterValue( Parameters::DIST_CUT_THRESH );
    distThreshold       = parameters.getRawParameterValue( Parameters::DIST_THRESHOLD );
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
    fuzz->setInputLevel( *distInputLevel );
    fuzz->setCutOff( *distCutoffThreshold );
    fuzz->setThreshold( *distThreshold );

    waveFolder->setInputLevel( *distInputLevel );
    // waveFolder->setCutOff( *distCutoffThreshold );
    waveFolder->setThreshold( *distThreshold );

    // int channelAmount = getTotalNumOutputChannels();
 
    // for ( int channel = 0; channel < channelAmount; ++channel ) {
    //     bool isOddChannel = channel % 2 == 0;


    //     lowPassFilters [ channel ]->setCoefficients( juce::IIRCoefficients::makeLowPass ( _sampleRate, *lowBand ));
    //     bandPassFilters[ channel ]->setCoefficients( juce::IIRCoefficients::makeBandPass( _sampleRate, *midBand, 1.0 ));
    //     highPassFilters[ channel ]->setCoefficients( juce::IIRCoefficients::makeHighPass( _sampleRate, *hiBand ));
    // }
}

/* resource management */

void AudioPluginAudioProcessor::prepareToPlay( double sampleRate, int samplesPerBlock )
{
    juce::ignoreUnused( samplesPerBlock );
    
    _sampleRate = sampleRate;

    // dispose previously allocated resources
    releaseResources();

    int channelAmount = getTotalNumOutputChannels();

    for ( int i = 0; i < channelAmount; ++i )
    {
        lowPassFilters.add ( new juce::IIRFilter());
        bandPassFilters.add( new juce::IIRFilter());
        highPassFilters.add( new juce::IIRFilter());

        lowPassFilters [ i ]->setCoefficients( juce::IIRCoefficients::makeLowPass ( sampleRate, Parameters::Config::LOW_BAND_DEF ));
        bandPassFilters[ i ]->setCoefficients( juce::IIRCoefficients::makeBandPass( sampleRate, Parameters::Config::MID_BAND_DEF, 1.0 ));
        highPassFilters[ i ]->setCoefficients( juce::IIRCoefficients::makeHighPass( sampleRate, Parameters::Config::HI_BAND_DEF ));
    }
    fuzz = new Fuzz( 1.0f );
    waveFolder = new WaveFolder( 1.0f );
    
    // align values with model
    updateParameters();
}

void AudioPluginAudioProcessor::releaseResources()
{
    lowPassFilters.clear();
    bandPassFilters.clear();
    highPassFilters.clear();

    if ( fuzz != nullptr ) {
        delete fuzz;
        fuzz = nullptr;
    }

    if ( waveFolder != nullptr ) {
        delete waveFolder;
        waveFolder = nullptr;
    }
}

/* rendering */

void AudioPluginAudioProcessor::processBlock( juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages )
{
    juce::ignoreUnused( midiMessages );
    juce::ScopedNoDenormals noDenormals;
  
    int channelAmount = buffer.getNumChannels();
    int bufferSize    = buffer.getNumSamples();
    
    float dryMix = 1.f - *dryWetMix;
    float wetMix = *dryWetMix;
    
    // // Create temporary buffers for each band
    // juce::AudioBuffer<float> lowBuffer( channelAmount, bufferSize );
    // juce::AudioBuffer<float> midBuffer( channelAmount, bufferSize );
    // juce::AudioBuffer<float> hiBuffer ( channelAmount, bufferSize );

    // Basic parameters
    const double sampleRate = getSampleRate();
    const float baseFreq = 110.0f;
    const int fftOrder = 11; // 2048-point FFT
    const int fftSize  = 1 << fftOrder;
    constexpr int hopSize  = fftSize / 2;

    static juce::dsp::FFT fft( fftOrder );

    // Static objects to avoid reallocations
    static std::vector<float> window(fftSize);
    static bool windowInit = false;
    if ( !windowInit )
    {
        for ( int n = 0; n < fftSize; ++n ) {
            window[ n ] = 0.5f - 0.5f * std::cos( 2.0 * juce::MathConstants<double>::pi * n / fftSize );
        }
        windowInit = true;
    }

    for ( int channel = 0; channel < channelAmount; ++channel )
    {
        if ( buffer.getReadPointer( channel ) == nullptr ) {
            continue;
        }
        /*

        lowBuffer.copyFrom ( channel, 0, buffer, channel, 0, bufferSize );
        midBuffer.copyFrom ( channel, 0, buffer, channel, 0, bufferSize );
        hiBuffer.copyFrom  ( channel, 0, buffer, channel, 0, bufferSize );

        // apply the effects

        // fuzz->apply( midBuffer, channel );
        waveFolder->apply( midBuffer, channel );

        // apply the filtering

        lowPassFilters [ channel ]->processSamples( lowBuffer.getWritePointer( channel ), bufferSize );
        bandPassFilters[ channel ]->processSamples( midBuffer.getWritePointer( channel ), bufferSize );
        highPassFilters[ channel ]->processSamples( hiBuffer.getWritePointer ( channel ), bufferSize );

        // write the effected buffer into the output
    
        for ( int i = 0; i < bufferSize; ++i ) {
            auto input = buffer.getSample( channel, i ) * dryMix;

            buffer.setSample(
                channel, i,
                input + (
                    lowBuffer.getSample( channel, i ) +
                    midBuffer.getSample( channel, i ) +
                    hiBuffer.getSample ( channel, i )
                ) * wetMix
            );
        }
        */
        auto* channelData = buffer.getWritePointer( channel );

        // Static state buffers per channel
        struct ChannelState
        {
            std::vector<float> inputBuffer;
            std::vector<float> outputBuffer;
            int writePos = 0;
            bool initialised = false;
        };
        static std::array<ChannelState, 2> state;

        auto& st = state[ channel ];
        if (!st.initialised)
        {
            st.inputBuffer.assign(fftSize, 0.0f);
            st.outputBuffer.assign(fftSize, 0.0f);
            st.initialised = true;
        }

        int samplesProcessed = 0;
        while ( samplesProcessed < bufferSize )
        {
            // Write new input into circular inputBuffer
            const int samplesToCopy = std::min(hopSize, bufferSize - samplesProcessed);
            std::memmove(
                st.inputBuffer.data(), st.inputBuffer.data() + hopSize, ( fftSize - hopSize ) * sizeof( float )
            );
            std::memcpy(
                st.inputBuffer.data() + ( fftSize - hopSize ), channelData + samplesProcessed, samplesToCopy * sizeof( float )
            );

            // --- Prepare FFT input with window ---
            std::vector<float> fftTime( fftSize * 2, 0.0f );
            for ( int i = 0; i < fftSize; ++i ) {
                fftTime[ i ] = st.inputBuffer[ i ] * window[ i ];
            }
            // --- Forward FFT ---
            fft.performRealOnlyForwardTransform( fftTime.data());

            // Convert to complex
            std::vector<std::complex<float>> spec( fftSize / 2 );
            for ( int i = 0; i < fftSize / 2; ++i ) {
                spec[ i ] = { fftTime[ 2 * i ], fftTime[ 2 * i + 1 ]};
            }
            // --- Split spectrum by harmonic proximity ---
            std::vector<std::complex<float>> specA( fftSize / 2 ), specB( fftSize / 2 );
            for ( int bin = 0; bin < fftSize / 2; ++bin )
            {
                float freq = bin * ( float ) sampleRate / fftSize;
                bool harmonic = false;
                for ( int n = 1; n <= 12; ++n ) {
                    float h = n * baseFreq;
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

            auto iFFT = [&](const std::vector<std::complex<float>>& src,
                            std::vector<float>& dst)
            {
                for ( int i = 0; i < fftSize / 2; ++i ) {
                    dst[ 2 * i ]     = src[ i ].real();
                    dst[ 2 * i + 1 ] = src[ i ].imag();
                }
                fft.performRealOnlyInverseTransform( dst.data() );
            };

            // --- iFFT and OLA accumulate ---
            std::vector<float> tempA( fftSize * 2, 0.0f ), tempB( fftSize * 2, 0.0f );
            iFFT( specA, tempA );
            iFFT( specB, tempB );

            // apply the distortion on the split buffers
            fuzz->apply( tempA.data(), tempA.size() );
            waveFolder->apply( tempB.data(), tempB.size() );

            // Sum and window
            for ( int i = 0; i < fftSize; ++i ) {
                st.outputBuffer[ i ] += ( tempA[ i ] + tempB[ i ]) * window[ i ];
            }

            // --- Write hopSize samples to output ---
            for ( int i = 0; i < hopSize && ( samplesProcessed + i < bufferSize ); ++i ) {
                channelData[ samplesProcessed + i ] = st.outputBuffer[ i ];
            }

            // Shift output buffer for next overlap
            std::memmove(
                st.outputBuffer.data(), st.outputBuffer.data() + hopSize, ( fftSize - hopSize ) * sizeof( float )
            );
            std::fill( st.outputBuffer.begin() + ( fftSize - hopSize ), st.outputBuffer.end(), 0.0f );

            samplesProcessed += hopSize;
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
    juce::ValueTree tree = juce::ValueTree::readFromData( data, static_cast<int>( sizeInBytes ));
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