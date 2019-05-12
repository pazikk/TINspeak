//
// Created by michael on 03.05.19.
//

#ifndef CLION_IAUDIOFRAMEPRODUCER_H
#define CLION_IAUDIOFRAMEPRODUCER_H

#include "AudioFrame.h"

class IAudioFrameProducer
{
public:
    virtual void AudioFrameProducer_NewData(AudioFrame *frame) = 0;
};

#endif //CLION_IAUDIOFRAMEPRODUCER_H
