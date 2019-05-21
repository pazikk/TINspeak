//
// Created by michael on 3/31/19.
//

#ifndef AUDIOGRABBER_H
#define AUDIOGRABBER_H

#include <alsa/asoundlib.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "IAudioFrameProducer.h"
#include "AudioSignalParams.h"
#include "AudioFrame.h"
#include <cstring>

class AudioGrabberALSA
{
public:
    enum InitParam
    {

        InitParam_Bitwidth_Int32 = 0,
        InitParam_Samplerate_Int32 = 1,
        InitParam_Channels_Int32 = 2,
        InitParam_DeviceNumber_Int32 = 3,
        InitParam_SamplesPerFrame_Int32 = 4,
        InitParam_IAudioDataGrabbed = 5,
    };

    struct TAudioGrabbnigDev
    {
        unsigned int nrDev;
        char  DevName[32];
    };

    AudioGrabberALSA();
    ~AudioGrabberALSA();

    void BeginInit() ;
    void SetParam(int param, int value);
    void SetParam(int param, void * value);
    void EndInit();
    void UnInit();

    static unsigned int GetNrOfGrabbingDevs();
    static unsigned int GetLstOfGrabbingDevs(TAudioGrabbnigDev *devList);

    void StartGrabbing();
    void StopGrabbing();

private:
    bool _initialized = false;
    bool _initInProgress = false;

    AudioSignalParams _initASignalParams;
    IAudioFrameProducer* _audioGrabbed = nullptr;
    int _deviceNumber = 0;

    bool _isRecording = false;

    void FillParameters();
    void Cleanup();
    void OpenDevice();

    // thread
    void RecordingJob();
    std::thread _recordingThread;

    // alsa
    snd_pcm_t *_alsaHandle;
    snd_pcm_hw_params_t *_alsaParams;
    snd_pcm_uframes_t _alsaFramesPerPeriod;
    snd_pcm_uframes_t _alsaFramesPerBuffer;
    int _alsaDir;
    char *_alsaBuffer;
    int _alsaBufferSize;
    int _alsaPeriodSize;
    int _alsaFrameSize;
    int _alsaPeriodsPerBuffer;
    unsigned int _alsaVal;
    unsigned int _alsaOverrunsCount;


};

#endif //AUDIOGRABBER_H