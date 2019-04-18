//
// Created by michael on 3/31/19.
//

#ifndef AUDIOSIGNALPARAMS_H
#define AUDIOSIGNALPARAMS_H
struct AudioSignalParams
{
    int BitPerSample;
    int NumberOfChannels;
    int SampleRate;
    int SamplesPerFrame;

    AudioSignalParams()
    {
        Reset();
    }

    void Reset()
    {
        BitPerSample = 0;
        NumberOfChannels = 0;
        SampleRate = 0;
        SamplesPerFrame = 0;
    }
};
#endif //AUDIOSIGNALPARAMS_H//

