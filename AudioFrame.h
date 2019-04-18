//
// Created by michael on 3/31/19.
//

#ifndef AUDIOFRAME_H
#define AUDIOFRAME_H

#include <stdint.h>

class AudioFrame
{
public:
    int StreamNo;
    unsigned char* Data;
    int DataSize;
    int NumberOfSamples;
    int64_t Ts;
};

#endif //AUDIOFRAME_H