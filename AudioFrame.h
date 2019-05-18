//
// Created by michael on 3/31/19.
//

#ifndef AUDIOFRAME_H
#define AUDIOFRAME_H
#include <cstring>
#include <cstdint>

class AudioFrame
{
public:
    AudioFrame()
    {
        Data = nullptr;
    }
    AudioFrame(int size)
    {
        isAllocated = true;
        Data = new unsigned char[size];
        DataSize = size;
    }
    // copy constructor
    AudioFrame(const AudioFrame &other)
    {
        isAllocated = true;
        Data = new unsigned char[other.DataSize];
        DataSize = other.DataSize;
        memcpy(Data, other.Data, DataSize);
    }
    ~AudioFrame()
    {
        if (isAllocated && Data != nullptr)
        {
            delete[] Data;
            Data = nullptr;
            isAllocated = false;
        }
    }
    uint32_t timestamp;
    int DataSize;
    unsigned char* Data;
    int NumberOfSamples; // initialized by opus after decoding
private:
    bool isAllocated = false;
};

#endif //AUDIOFRAME_H