//
// Created by michael on 03.05.19.
//

#ifndef CLION_ENCODEDAUDIO_H
#define CLION_ENCODEDAUDIO_H

#include <cstring>

class EncodedAudio
{
public:
    unsigned char *Data;
    unsigned int DataSize;
    unsigned int FrameCount;
    bool isValid = true;

    EncodedAudio()
    {
        Data = nullptr;
        DataSize = 0;
    }
    // this is used while adding to sendingQueue
    EncodedAudio(const EncodedAudio &other)
    {
        Data = new unsigned char [other.DataSize];
        std::memcpy(Data, other.Data, other.DataSize);
        DataSize = other.DataSize;
        FrameCount = other.FrameCount;
        isAllocated = true;
    }
    EncodedAudio& operator=(const EncodedAudio &other)
    {
        if (isAllocated == false)
        {
            isAllocated = true;
            Data = new unsigned char [other.DataSize];
            std::memcpy(Data, other.Data, other.DataSize);
        }
        DataSize = other.DataSize;
        FrameCount = other.FrameCount;
        isAllocated = true;
        return *this;
    }

    ~EncodedAudio()
    {
        if (isAllocated && Data != nullptr)
        {
            delete [] Data;
            Data = nullptr;
            isAllocated = false;
        }
    }

    bool isAllocated = false;
};

#endif //CLION_ENCODEDAUDIO_H
