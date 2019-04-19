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
// 44100 is supported by ALSA, but not by Opus
#define SAMPLE_RATE 48000
// use stereo for better results
#define CHANNELS_COUNT 2
// 16 and 32 works, 24 not supported, 8 is buggy (ALSA)
// 16 is optimal
#define BITS_PER_SAMPLE 16

using namespace std::chrono_literals;

class Audio
{

public:
    Audio()
    {
        int captureDeviceNr = 0;
        int playbackDeviceNr = 0;

        _replay = new AudioReplayALSA();
        _grabber = new AudioGrabberALSA();

        ListInDevs();
        std::cout << "Choose capture device: ";
        std::cin >> captureDeviceNr;
        if (captureDeviceNr > _grabber->GetNrOfGrabbingDevs() || captureDeviceNr < 0)
            throw std::runtime_error("Specified capture device does not exist.");

        ListOutDevs();
        std::cout << "Choose playback device: ";
        std::cin >> playbackDeviceNr;
        if (playbackDeviceNr > _replay->GetNrOfReplayDevs() || playbackDeviceNr < 0)
            throw std::runtime_error("Specified replay device does not exist.");

        InitAudioReplay(_replay, playbackDeviceNr);
        InitAudioGrabber(_grabber, captureDeviceNr);

        // starting grabbing thread, record to file for 3 seconds, delete grabber to close test.raw file
        _grabber->StartGrabbing();
        std::this_thread::sleep_for(3s);
        _grabber->StopGrabbing();
        _grabber->UnInit();
        _grabber = nullptr;

        // playing recorded audio from file
        _fileToReadDesc = fopen("test.raw","rb");
        int bufferSize = FRAMES_COUNT * CHANNELS_COUNT * (BITS_PER_SAMPLE/8);
        auto buffer = (unsigned char*)malloc(bufferSize);
        printf("Proceeding to play...\n");
        float bytesPlayed = 0;
        int bytesRead = 0;
        while ((bytesRead = fread(buffer, sizeof(char), bufferSize, _fileToReadDesc)) > 0)
        {
            bytesPlayed += bytesRead;
            if (bytesRead != bufferSize)
                std::cout << "Short read from file.\n";
            AudioFrame af;
            af.Data = buffer;
            af.DataSize = bufferSize;
            af.NumberOfSamples = FRAMES_COUNT;
            _replay->Replay(&af);
        }
        printf("Playing ended. Played %f bytes.\n", bytesPlayed);

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


    static void InitAudioGrabber(AudioGrabberALSA* ag, int devNr)
    {
        ag->BeginInit();
        ag->SetParam(AudioGrabberALSA::InitParam_DeviceNumber_Int32, devNr);
        ag->SetParam(AudioGrabberALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        ag->SetParam(AudioGrabberALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        ag->SetParam(AudioGrabberALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        ag->SetParam(AudioGrabberALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        ag->EndInit();
    }
    static void InitAudioReplay(AudioReplayALSA* ar, int devNr)
    {
        ar->BeginInit();
        ar->SetParam(AudioReplayALSA::InitParam_DeviceNumber_Int32, devNr);
        ar->SetParam(AudioReplayALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        ar->SetParam(AudioReplayALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        ar->SetParam(AudioReplayALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        ar->SetParam(AudioReplayALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        ar->EndInit();
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
        unsigned int cnt = _replay->GetNrOfReplayDevs();
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