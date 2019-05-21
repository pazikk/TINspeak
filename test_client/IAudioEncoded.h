//
// Created by michael on 03.05.19.
//

#ifndef CLION_IAUDIOENCODED_H
#define CLION_IAUDIOENCODED_H
#include "EncodedAudio.h"

class IAudioEncoded
{
public:
    virtual void AudioEncoded(EncodedAudio* audioPacket) = 0;
};

#endif //CLION_IAUDIOENCODED_H
