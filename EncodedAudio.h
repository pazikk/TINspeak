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
    uint16_t authorPort = 0;

};

#endif //CLION_ENCODEDAUDIO_H
