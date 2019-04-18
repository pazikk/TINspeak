//
// Created by michael on 18.04.19.
//

#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <alsa/asoundlib.h>
#include "AudioGrabberALSA.h"
#include "AudioReplayALSA.h"
#include "AudioFrame.h"

#define FRAMES_COUNT 240
#define SAMPLE_RATE 48000
// only stereo seems to be working on ALSA atm
#define CHANNELS_COUNT 2
// 16 and 32 works, 24 not supported, 8 is buggy (ALSA)
#define BITS_PER_SAMPLE 16

using namespace std::chrono_literals;

class Audio
{

public:
    Audio()
    {
        ListInDevs();
        ListOutDevs();

        _replay = new AudioReplayALSA();
        _grabber = new AudioGrabberALSA();

        InitAudioReplay(5);
        InitAudioGrabber(_grabber, 0);
        _grabber->StartGrabbing();
        std::this_thread::sleep_for(3s);
        _grabber->StopGrabbing();
        _grabber->UnInit();
        _grabber = nullptr;

        _fileToReadDesc = fopen("test.raw","rb");
        int frameSize = FRAMES_COUNT * CHANNELS_COUNT * (BITS_PER_SAMPLE/8);
        unsigned char* buffer = (unsigned char*)malloc(frameSize);
        printf("Proceeding to play:\n");
        while ((fread(buffer, sizeof(char), frameSize, _fileToReadDesc) > 0))
        {
            AudioFrame af;
            af.Data = buffer;
            af.DataSize = frameSize;
            af.NumberOfSamples = FRAMES_COUNT;
            _replay->Replay(&af);


        }
        printf("Playing ended.\n");

    }
    ~Audio()
    {
        if (_grabber != nullptr)
        {
            _grabber->StopGrabbing();
            _grabber->UnInit();
            _grabber = nullptr;
        }

        if (_replay != nullptr)
        {
            _replay->UnInit();
            _replay = nullptr;
        }

        if (_fileToReadDesc != nullptr)
        {
            fclose(_fileToReadDesc);
            _fileToReadDesc = nullptr;
        }
    }

private:
    AudioGrabberALSA* _grabber = nullptr;
    AudioReplayALSA* _replay = nullptr;
    FILE * _fileToReadDesc;


    void InitAudioGrabber(AudioGrabberALSA* ag, int devNr)
    {
        ag->BeginInit();
        ag->SetParam(AudioGrabberALSA::InitParam_DeviceNumber_Int32, devNr);
        ag->SetParam(AudioGrabberALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        ag->SetParam(AudioGrabberALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        ag->SetParam(AudioGrabberALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        ag->SetParam(AudioGrabberALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        ag->EndInit();
    }
    void InitAudioReplay(int devNr)
    {
        _replay->BeginInit();
        _replay->SetParam(AudioReplayALSA::InitParam_DeviceNumber_Int32, devNr);
        _replay->SetParam(AudioReplayALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        _replay->SetParam(AudioReplayALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        _replay->SetParam(AudioReplayALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        _replay->SetParam(AudioReplayALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        _replay->EndInit();
    }

    void ListInDevs()
    {
        unsigned int cnt = _grabber->GetNrOfGrabbingDevs();
        printf("Audio dev cnt: %u\n", cnt);
        AudioGrabberALSA::TAudioGrabbnigDev devList[32];
        unsigned int c = _grabber->GetLstOfGrabbnigDevs(devList, 32);

        for (unsigned int i = 0; i < c; i++)
        {
            printf("%u -> %s\n", devList[i].nrDev, devList[i].DevName);
        }
    }
    void ListOutDevs()
    {
        unsigned int cnt = _replay->GetNrOfRepalyDevs();
        printf("Audio out dev cnt: %u\n", cnt);
        AudioReplayALSA::AudioGrabbnigDev devList[32];
        unsigned int c = _replay->GetLstOfReplayDevs(devList, 32);
        for (unsigned int i = 0; i < c; i++)
        {
            printf("%u -> %s\n", devList[i].DevId, devList[i].DevName);
        }
    }

};

int main()
{
    Audio test;

    return 0;
}