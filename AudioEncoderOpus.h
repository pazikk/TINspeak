//
// Created by michael on 03.05.19.
//

#ifndef CLION_AUDIOENCODEROPUS_H
#define CLION_AUDIOENCODEROPUS_H

#include "AudioFrame.h"
#include "IAudioEncoded.h"
#include "opus/opus.h"
#include <vector>

#define MAX_PACKET_SIZE 4000
#define APPLICATION OPUS_APPLICATION_AUDIO

class AudioEncoderOpus
{
public:
    enum InitParam
    {
        InitParam_IAudioEncoded = 0,
        InitParam_Int32_NumberOfChannels,
        InitParam_Int32_SampleRate,
        InitParam_Int32_BitPerSample,
        InitParam_Int32_TargetBitrateInKbps,
    };

    AudioEncoderOpus();
    ~AudioEncoderOpus();
    void BeginInit();
    void SetParam(int param, int value);

    void SetParam(int param, void *value);
    void EndInit();

    void UnInit();
    void Release();

    void Encode(AudioFrame *frame);

    int GetFrameSize();

    inline bool IsFrameSizeValid(int frameSize);

    bool _initialized;
    bool _initInProgress;

    IAudioEncoded* _audioEncoded;
    int _numberOfChannels;
    int _sampleRate;
    int _bitPerSample;
    int _frameSize;
    int _targetBitrateInKbps;

    // opus
    std::vector<opus_int16> _opusIn;
    OpusEncoder *_opusEncoderPtr;

};

#endif //CLION_AUDIOENCODEROPUS_H
