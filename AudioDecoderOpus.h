//
// Created by michael on 03.05.19.
//

#ifndef CLION_AUDIODECODEROPUS_H
#define CLION_AUDIODECODEROPUS_H

#include <opus/opus.h>

#include <vector>
#include <cstring>
#include "AudioFrame.h"
#include "IAudioDecoded.h"
#include "EncodedAudio.h"

#define APPLICATION OPUS_APPLICATION_AUDIO
#define DEFAULT_FRAME_SIZE 480
#define MAX_FRAME_SIZE 2880
#define MAX_PACKET_SIZE 4000

class AudioDecoderOpus
{
public:

    enum InitParam
    {
        InitParam_IAudioDecoded,
        InitParam_Int32_NumberOfChannels,	// signifying number of channels in bitstream
        InitParam_Int32_SampleRate,			// Sampling rate of the bitstream
        InitParam_Int32_BitPerSample,		// shuld we use sample type enum insted of it?
    };

    AudioDecoderOpus();
    ~AudioDecoderOpus();
    void BeginInit();
    void SetParam(int param, int value);
    void SetParam(int param, void *value);
    void EndInit();

    void UnInit();

    void Decode(EncodedAudio* ea);

    int GetFrameSize();
private:
    bool _initialized;
    bool _initInProgress;
    int _frameSize;

    IAudioDecoded *_audioDecoded;
    int _numberOfChannels;
    int _sampleRate;
    int _bitPerSample;

    // opus
    OpusDecoder *_opusDecoderPtr;
    opus_int16 *_opusOut;
    std::vector<unsigned char> _decodedData;

    void InitDecoder();
};



#endif //CLION_AUDIODECODEROPUS_H
