//
// Created by michael on 18.04.19.
//

#ifndef CLION_AUDIOREPLAYALSA_H
#define CLION_AUDIOREPLAYALSA_H

#include "AudioSignalParams.h"
#include <alsa/asoundlib.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <queue>
#include <map>
#include <stdexcept>
#include "AudioFrame.h"

#define MIN_READY_FRAMES 10

class AudioReplayALSA
{
public:
    enum InitParam
    {
        InitParam_Bitwidth_Int32 = 0,				//bits on one sample
        InitParam_Samplerate_Int32 = 1,				//number of samples per second
        InitParam_Channels_Int32 = 2,				//number of channels
        InitParam_DeviceNumber_Int32 = 3, 			//number of used audio source
        InitParam_SamplesPerFrame_Int32 = 4,		//number of samples in one frame delivered by callback
    };

    struct AudioGrabbnigDev
    {
        unsigned int DevId;
        char DevName[32];
    };

    AudioReplayALSA();
    ~AudioReplayALSA();

    void BeginInit();
    void SetParam(int param, int value);
    void SetParams(AudioSignalParams& audioSignalParams);
    void EndInit();
    void UnInit();
    void Release();

    unsigned int GetNrOfReplayDevs();
    unsigned int GetLstOfReplayDevs(AudioGrabbnigDev *listOfDev, unsigned int listOfDevLenght);

    void QueueToReplay(AudioFrame *frame);
    void StartReplay();
    void StopReplay();



private:
    bool _initialized;
    bool _initInProgress;
    bool _isPlaying;

    std::map<uint16_t, std::queue<AudioFrame>> _mapOfQueues;

    void Cleanup();
    void OpenDevice();
    void FillParameters();

    void PlayingJob();
    std::thread _playingThread;
    std::mutex _mut;
    std::condition_variable _cond;

    AudioSignalParams _signalParams;
    int _deviceNumber = 0;

    // alsa
    snd_pcm_t *_alsaHandle;
    snd_pcm_hw_params_t *_alsaParams;
    snd_pcm_uframes_t _alsaFramesPerPeriod;
    snd_pcm_uframes_t _alsaFramesPerBuffer;
    int _alsaPeriodsPerBuffer;
    int _alsaDir;
    char *_alsaBuffer;
    int _alsaBufferSize;
    int _alsaPeriodSize;
    int _alsaFrameSize;
    unsigned int _alsaVal;
    FILE * _fileToWriteDesc;
    unsigned int _alsaUnderrunsCount;
    uint32_t _framesInQueuesNumber = 0;

    int16_t mix_sample(int16_t sample1, int16_t sample2);
    AudioFrame mix_frames(AudioFrame &frame1, AudioFrame &frame2);

};


#endif //CLION_AUDIOREPLAYALSA_H
