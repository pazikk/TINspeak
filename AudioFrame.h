//
// Created by michael on 3/31/19.
//

#ifndef AUDIOFRAME_H
#define AUDIOFRAME_H

class AudioFrame
{
public:
    unsigned char* Data;
    int DataSize;
    int NumberOfSamples;
};

#endif //AUDIOFRAME_H