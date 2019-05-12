//
// Created by michael on 03.05.19.
//

#include "AudioEncoderOpus.h"
#include <exception>
#include <stdexcept>


AudioEncoderOpus::AudioEncoderOpus()
{
    _initialized = false;
    _initInProgress = false;

    _audioEncoded = nullptr;
    _numberOfChannels = 2;
    _sampleRate = 48000;
    _bitPerSample = 16;
    _frameSize = 0;
    _targetBitrateInKbps = 64;

    _opusEncoderPtr = nullptr;
}
AudioEncoderOpus::~AudioEncoderOpus()
{
    UnInit();
}
void AudioEncoderOpus::BeginInit()
{
    if (_initialized) throw std::runtime_error("AudioEncoderOpus Module is already initialized");
    if (_initInProgress) throw std::runtime_error("AudioEncoderOpus Module initialization is already in progress");
    _initInProgress = true;

    _numberOfChannels = 2;
    _sampleRate = 0;
    _bitPerSample = 0;
    _targetBitrateInKbps = 0;
}
void AudioEncoderOpus::SetParam(int param, int value)
{
    switch ((InitParam)param)
    {
        case InitParam_Int32_NumberOfChannels:
            _numberOfChannels = value;
            printf("SetParam: NumberOfChannels = %d", _numberOfChannels);
            break;
        case InitParam_Int32_SampleRate:
            _sampleRate = value;
            printf("SetParam: SampleRate = %d", _sampleRate);
            break;
        case InitParam_Int32_BitPerSample:
            _bitPerSample = value;
            printf("SetParam: BitPerSample = %d", _bitPerSample);
            break;
        case InitParam_Int32_TargetBitrateInKbps:
            _targetBitrateInKbps = value;
            printf("SetParam: TargetBitrate = %dkbps", _targetBitrateInKbps);
            break;
        default:
            printf("SetParam(int): UnknownParam = %d", value);
    }
}
void AudioEncoderOpus::SetParam(int param, void *value)
{
    switch ((InitParam)param)
    {
        case InitParam_IAudioEncoded:
            _audioEncoded = (IAudioEncoded*)value;
            printf("SetParam: AudioEncoded  Callback = 0x%X", _audioEncoded);
            break;
        default:
            printf("SetParam(*void): UnknownParam = 0x%X.", value);
    }
}
void AudioEncoderOpus::EndInit()
{
    int err;
    if (!_initInProgress) throw std::runtime_error("BeginInit needs to be called first. (BeforeEndInit).");
    _initInProgress = false;

    bool initParamsError = false;
    if (_audioEncoded == nullptr)
    {
        printf("Audio encoded interface must be set.");
        initParamsError = true;
    }
    if (_numberOfChannels > 2)
    {
        printf("Invalid Number of channels(%d). Supported only two chanel audio.", _numberOfChannels);
        initParamsError = true;
    }
    if ((_sampleRate != 8000) && (_sampleRate != 12000) && (_sampleRate != 16000) && (_sampleRate != 32000) && (_sampleRate != 48000))
    {
        printf("Invalid sample rate(%d). Supported sample rates: 8000, 16000, 32000, 48000.", _sampleRate);
        initParamsError = true;
    }
    if (_bitPerSample != 16)
    {
        printf("Invalid bit per sample(%d). Supported only 16bit signed short samples.", _bitPerSample);
        initParamsError = true;
    }
    if (_targetBitrateInKbps < 5 || _targetBitrateInKbps > 512)
    {
        printf("Invalid bitrate (%d). Supported range is from 5 kbps to 512 kbps.", _bitPerSample);
        initParamsError = true;
    }
    if (initParamsError) throw std::runtime_error("Lack or invalid init parameters.");

    _opusEncoderPtr = opus_encoder_create(_sampleRate, _numberOfChannels, APPLICATION, &err);
    if (err<0)
    {
        printf("failed to create an encoder: %s\n", opus_strerror(err));
        throw std::runtime_error("Problem with encoder (check log for details).");
    }

    err = opus_encoder_ctl(_opusEncoderPtr, OPUS_SET_BITRATE(_targetBitrateInKbps * 1000));
    if (err<0)
    {
        printf("failed to set bitrate: %s\n", opus_strerror(err));
    }

    err = opus_encoder_ctl(_opusEncoderPtr, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    if (err<0)
    {
        printf("failed to set signal type to voice: %s\n", opus_strerror(err));
    }

    if (err) throw std::runtime_error("Couldnt Start opus encoder with those settings.");
    _initialized = true;
}
void AudioEncoderOpus::UnInit()
{

    if (_opusEncoderPtr != nullptr)
    {
        opus_encoder_destroy(_opusEncoderPtr);
        _opusEncoderPtr = nullptr;
    }
    _initialized = false;
    printf("AudioEncoderOpus uninitialized");
}

void AudioEncoderOpus::Encode(AudioFrame* frame)
{
    if (!_initialized) throw std::runtime_error("AudioEncoderOpus Module is not initialized.\n");

    if (frame->Data == nullptr)
    {
        // flush
        printf("Frame data pointer is null.\n");
        return;
    }
    if (IsFrameSizeValid(frame->NumberOfSamples))
    {
        _frameSize = frame->NumberOfSamples;
    }
    else
    {
        throw std::runtime_error("Unsupported frame size. Supported are: 2.5, 5, 10, 20, 40, 60 ms.");
    }

    _frameSize = frame->NumberOfSamples;
    unsigned char encodedData[MAX_PACKET_SIZE];

    if (frame->NumberOfSamples * _numberOfChannels > _opusIn.size())
    {
        _opusIn.resize(frame->NumberOfSamples * _numberOfChannels);
    }
    // Convert from little-endian ordering.
    for (int i = 0; i < _opusIn.size(); i++)
    {
        _opusIn[i] = frame->Data[2 * i + 1] << 8 | frame->Data[2 * i];
    }
    int ebc = opus_encode(_opusEncoderPtr, &_opusIn[0], frame->NumberOfSamples, encodedData, MAX_PACKET_SIZE);
    if (ebc < 0)
    {
        printf("Opus audio encode failed: %s\n", opus_strerror(ebc));
    }
        //else if (ebc <= 2)
        //{
        //	// If encodedBytesCount is 2 bytes or less, then the packet does not need to be transmitted (DTX).
        //	return;
        //}
    else
    {
        EncodedAudio ea;
        ea.Data = encodedData;
        ea.DataSize = ebc;

        _audioEncoded->AudioEncoded(&ea);
    }
}

int AudioEncoderOpus::GetFrameSize()
{
    return _frameSize;
}
inline bool AudioEncoderOpus::IsFrameSizeValid(int frameSize)
{
    double timeInMs = static_cast<double>(frameSize) / _sampleRate * 1000;
    if (timeInMs != 2.5 && timeInMs != 5 && timeInMs != 10 && timeInMs != 20 && timeInMs != 40 && timeInMs != 60)
    {
        printf("Invalid frame size: %f", timeInMs);
        return false;
    }
    else
    {
        return true;
    }
}





