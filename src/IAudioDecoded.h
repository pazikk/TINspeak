//
// Created by michael on 03.05.19.
//

#ifndef CLION_IAUDIODECODED_H
#define CLION_IAUDIODECODED_H

#include "AudioFrame.h"

class IAudioDecoded
{
public:
    virtual void AudioDecoded(AudioFrame *frame) = 0;
};

#endif //CLION_IAUDIODECODED_H
