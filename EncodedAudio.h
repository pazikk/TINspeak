//
// Created by michael on 03.05.19.
//

#ifndef CLION_ENCODEDAUDIO_H
#define CLION_ENCODEDAUDIO_H

struct EncodedAudio
{
    unsigned char *Data;
    unsigned int DataSize;
    unsigned int FrameCount;

    EncodedAudio()
    {
        Data = nullptr;
        DataSize = 0;
    }
};

#endif //CLION_ENCODEDAUDIO_H
