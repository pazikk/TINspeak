//
// Created by michael on 03.05.19.
//

#include "AudioDecoderOpus.h"
#include <exception>
#include <stdexcept>
#include "AudioDecoderOpus.h"


AudioDecoderOpus::AudioDecoderOpus()
{
    _initialized = false;
    _initInProgress = false;
    _frameSize = DEFAULT_FRAME_SIZE;

    _numberOfChannels = 2;
    _sampleRate = 0;
    _bitPerSample = 0;

    _audioDecoded = nullptr;

    _opusDecoderPtr = nullptr;
    _opusOut = nullptr;

}
AudioDecoderOpus::~AudioDecoderOpus()
{
    UnInit();
}
void AudioDecoderOpus::BeginInit()
{
    if (_initialized) throw std::runtime_error("AudioDecoderOpus module is already initialized");
    if (_initInProgress) throw std::runtime_error("AudioDecoderOpus module initialization is already in progress");
    _initInProgress = true;

    _audioDecoded = nullptr;
    _numberOfChannels = 2;
    _sampleRate = 0;
    _bitPerSample = 0;
}
void AudioDecoderOpus::SetParam(int param, int value)
{
    switch ((InitParam)param)
    {
        case InitParam_Int32_NumberOfChannels:
            _numberOfChannels = value;
            printf("SetParam: _numberOfChannels = %d", _numberOfChannels);
            break;
        case InitParam_Int32_SampleRate:
            _sampleRate = value;
            printf("SetParam: _sampleRate = %d", _sampleRate);
            break;
        case InitParam_Int32_BitPerSample:
            _bitPerSample = value;
            printf("SetParam: _bitPerSample = %d", _bitPerSample);
            break;
        default:
            printf("SetParam(int): UnknownParam = %d", value);
    }
}
void AudioDecoderOpus::SetParam(int param, void *value)
{
    switch ((InitParam)param)
    {
        case InitParam_IAudioDecoded:
            _audioDecoded = (IAudioDecoded*)value;
            printf("SetParam: AudioDecoded  Callback = 0x%X", _audioDecoded);
            break;
        default:
            printf("SetParam(*void): UnknownParam = 0x%X.", value);
    }
}
void AudioDecoderOpus::EndInit()
{
    if (!_initInProgress) throw std::runtime_error("Module BeginInit was not called");
    _initInProgress = false;

    bool initParamsError = false;
    if (_audioDecoded == nullptr)
    {
        printf("Audio decoded interface must be set.");
        initParamsError = true;
    }
    if (_numberOfChannels != 2 && _numberOfChannels != 1)
    {
        printf("Invalid Number of chanels(%d). Supported only single or dual channel audio.", _numberOfChannels);
        initParamsError = true;
    }
    if ((_sampleRate != 8000) && (_sampleRate != 12000) && (_sampleRate != 16000) && (_sampleRate != 32000) && (_sampleRate != 48000))
    {
        printf("Invalid sample rate(%d). Supported sample rates: 8000, 16000, 24000, 32000, 48000.", _sampleRate);
        initParamsError = true;
    }
    if (_bitPerSample != 16)
    {
        printf("Invalid bit per sample(%d). Supported only 16bit signed short samples.", _bitPerSample);
        initParamsError = true;
    }

    if (initParamsError) throw std::runtime_error("Lack or invalid init parameters.");

    InitDecoder();
    _opusOut = new opus_int16[MAX_FRAME_SIZE * _numberOfChannels];
    _initialized = true;
}
void AudioDecoderOpus::UnInit()
{
    if (_opusDecoderPtr != nullptr)
    {
        opus_decoder_destroy(_opusDecoderPtr);
        _opusDecoderPtr = nullptr;
    }
    if (_opusOut != nullptr)
    {
        delete[] _opusOut;
        _opusOut = nullptr;
    }
    printf("AudioDecoderOpus uni1nitialized");
    _initialized = false;
}

void AudioDecoderOpus::Decode(EncodedAudio* ea)
{
    if (!_initialized)
    {
        printf("Tired to decode received frame before initializing decoder. Frame dropped.\n");
        return;
    }
    if (ea == nullptr)
    {
        return;
    }


    _frameSize = opus_decode(_opusDecoderPtr, ea->Data, ea->DataSize, &_opusOut[0], MAX_FRAME_SIZE, 0);
    if (_frameSize<0)
    {
        fprintf(stderr, "decoder failed: %s\n", opus_strerror(_frameSize));
        return;
    }
    if (_decodedData.size() < _frameSize * _numberOfChannels * 2)
    {
        _decodedData.resize(_frameSize * _numberOfChannels * 2);
    }
    /* Convert to little-endian ordering. */
    for (int i = 0; i< _numberOfChannels * _frameSize; i++)
    {
        _decodedData[2 * i] = _opusOut[i] & 0xFF;
        _decodedData[2 * i + 1] = (_opusOut[i] >> 8) & 0xFF;
    }

    AudioFrame f;
    f.Data = &_decodedData[0];
    f.NumberOfSamples = _frameSize;
    f.DataSize = _frameSize * _numberOfChannels * 2;  // * 2, because 16 bits per sample, that is 2 bytes
    _audioDecoded->AudioDecoded(&f);
}
int AudioDecoderOpus::GetFrameSize()
{
    return _frameSize;
}
void AudioDecoderOpus::InitDecoder()
{
    int err;
    _opusDecoderPtr = opus_decoder_create(_sampleRate, _numberOfChannels, &err);
    if (err < 0)
    {
        printf("failed to create decoder: %s\n", opus_strerror(err));
        throw std::runtime_error("Cannot proceed without decoder.");
    }
}






