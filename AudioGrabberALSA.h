//
// Created by michael on 3/31/19.
//

#ifndef AUDIOGRABBER_H
#define AUDIOGRABBER_H

#include <alsa/asoundlib.h>
#include <thread>
#include <chrono>
#include <iostream>
#include "AudioSignalParams.h"
#include "AudioFrame.h"

class AudioGrabberALSA
{
public:
    enum InitParam
    {

        InitParam_Bitwidth_Int32 = 0,			//bits on one sample
        InitParam_Samplerate_Int32 = 1,			//number of samples per second
        InitParam_Channels_Int32 = 2,			//number of channels
        InitParam_DeviceNumber_Int32 = 3, 		//number of used audio source, not used if DeviceName is set
        InitParam_SamplesPerFrame_Int32 = 4,		//number of samples in one frame delivered by callback
        InitParam_IAudioDataGrabbed = 5,
        InitParam_AutoDetection_Int32 = 6,		//0 if disabled
        InitParam_ServerPort_Int32 = 7,
        InitParam_ServerName_Pchar = 8,			// char *
        InitParam_DeviceName_Pchar = 9,			// if set used insteed of DeviceNumber

        InitParam_ConsultationId_Pchar = 100,
    };

    struct TAudioGrabbnigDev
    {
        unsigned int nrDev;
        char  DevName[32];// 32 because WinApi szPname is max 32 long
    };

    AudioGrabberALSA();
    ~AudioGrabberALSA();

    void BeginInit() ;
    void SetParam(int param, int value);
    void SetParam(int param, double value);
    void SetParam(int param, void * value);
    void EndInit();
    void UnInit();
    void Release();

    unsigned int GetNrOfGrabbingDevs();
    unsigned int GetLstOfGrabbnigDevs(TAudioGrabbnigDev* devList, unsigned int devListSize);

    void StartGrabbing();
    void StopGrabbing();
    bool IsGrabbing();

private:
    bool _initialized = false;
    bool _initInProgress = false;


    AudioSignalParams _initASignalParams;
    //IAudioFrameProducer* _audioGrabbed = nullptr;
    bool _autoDetection = false;
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