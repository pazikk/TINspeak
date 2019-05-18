//
// Created by michael on 03.05.19.
//

#ifndef CLION_ENCODEDAUDIO_H
#define CLION_ENCODEDAUDIO_H

#include <cstdint>

struct EncodedAudio
{
    unsigned char *Data = nullptr;
    unsigned int DataSize = 0;
    unsigned int FrameCount;
    uint32_t timestamp;

};

#endif //CLION_ENCODEDAUDIO_H
