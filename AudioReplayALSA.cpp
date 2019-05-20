//
// Created by michael on 18.04.19.
//
#include "AudioReplayALSA.h"

AudioReplayALSA::AudioReplayALSA()

        : _alsaBuffer(nullptr),
          _alsaPeriodsPerBuffer(2),
          _alsaBufferSize(0),
          _alsaDir(0),
          _alsaHandle(nullptr),
          _alsaParams(nullptr),
          _alsaFramesPerBuffer(0),
          _alsaFramesPerPeriod(0),
          _alsaVal(0),
          _alsaUnderrunsCount(0),

          _fileToWriteDesc(nullptr),

          _initialized(false),
          _initInProgress(false),
          _isPlaying(false)
{
    _signalParams.BitPerSample = 16;
    _signalParams.NumberOfChannels = 2;
    _signalParams.SampleRate = 48000;
    _signalParams.SamplesPerFrame = _signalParams.SampleRate / 25;
}
AudioReplayALSA::~AudioReplayALSA()
{
    UnInit();
}
void AudioReplayALSA::BeginInit()
{
    if (_initialized) throw std::runtime_error("AudioReplay module is already initialized");
    if (_initInProgress) throw std::runtime_error("AudioReplay module initialization is already in progress");
    _initInProgress = true;
    printf( "ALSA library version: %s\n", SND_LIB_VERSION_STR);
}
void AudioReplayALSA::SetParam(int param, int value)
{
    switch ((InitParam)param)
    {
        case InitParam_Bitwidth_Int32:
            _signalParams.BitPerSample = value;
            printf( "SetParam: BitPerSample = %d.", _signalParams.BitPerSample);
            break;
        case InitParam_Samplerate_Int32:
            _signalParams.SampleRate = value;
            printf( "SetParam: Samplerate = %d.", _signalParams.SampleRate);
            break;
        case InitParam_Channels_Int32:
            _signalParams.NumberOfChannels = value;
            printf( "SetParam: NumberOfChannels = %d.", _signalParams.NumberOfChannels);
            break;
        case InitParam_DeviceNumber_Int32:
            _deviceNumber = value;
            printf( "SetParam: _deviceNumber = %d.", _deviceNumber);
            break;
        case InitParam_SamplesPerFrame_Int32:
            _signalParams.SamplesPerFrame = value;
            printf( "SetParam: SamplesPerFrame = %d.", _signalParams.SamplesPerFrame);
            break;
        default:
            printf( "SetParam: UnknownParam = %d", value);
    }
}
void AudioReplayALSA::SetParams(AudioSignalParams &audioSignalParams)
{
    _signalParams = audioSignalParams;
}
void AudioReplayALSA::EndInit()
{
    if (!_initInProgress) throw std::runtime_error("Module BeginInit was not called");

    OpenDevice();
    FillParameters();

    _alsaFrameSize = (_signalParams.BitPerSample / 8) * _signalParams.NumberOfChannels;
    _alsaBufferSize = _alsaPeriodSize * _alsaPeriodsPerBuffer;
    _alsaBuffer = (char *)malloc(_alsaPeriodSize); // TODO maybe not malloc? (outdated)

    _initialized = true;
    _initInProgress = false;
}
void AudioReplayALSA::UnInit()
{
    StopReplay();
    Cleanup();
    printf("AudioReplayALSA uninitialized.\n");
    _initialized = false;
}
void AudioReplayALSA::Release()
{
    delete this;
}
unsigned int AudioReplayALSA::GetNrOfReplayDevs()
{
    int deviceCount = 0;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0)
    {
        int err = 0;
        snd_ctl_t *handle;
        char name[20];
        snprintf(name, sizeof(name), "hw:%d", card);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0)
        {
            printf("Couldn't open this device: %s , ALSA error code: %d", name, err);
            continue;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0)
        {
            printf("Couldn't get card info about: %s , ALSA error code: %d", name, err);
            snd_ctl_close(handle);
            continue;
        }
        int dev = -1;
        while (snd_ctl_pcm_next_device(handle, &dev) >= 0 && dev >= 0)
        {
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
            {
                continue;
            }
            deviceCount++;
        }
        snd_ctl_close(handle);
    }
    return deviceCount;
}
unsigned int AudioReplayALSA::GetLstOfReplayDevs(AudioGrabbnigDev *listOfDev, unsigned int listOfDevLenght)
{
    int deviceCount = 0;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);

    int card = -1;
    while (snd_card_next(&card) >= 0 && card >= 0)
    {
        int err = 0;
        snd_ctl_t *handle;
        char name[20];
        snprintf(name, sizeof(name), "hw:%d", card);
        if ((err = snd_ctl_open(&handle, name, 0)) < 0)
        {
            printf("Couldn't open this device: %s , ALSA error code: %d", name, err);
            continue;
        }
        if ((err = snd_ctl_card_info(handle, info)) < 0)
        {
            printf("Couldn't get card info about: %s , ALSA error code: %d", name, err);
            snd_ctl_close(handle);
            continue;
        }
        int dev = -1;
        while (snd_ctl_pcm_next_device(handle, &dev) >= 0 && dev >= 0)
        {
            snd_pcm_info_set_device(pcminfo, dev);
            snd_pcm_info_set_subdevice(pcminfo, 0);
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
            if (snd_ctl_pcm_info(handle, pcminfo) < 0)
            {
                continue;
            }
            char szDeviceID[20];
            snprintf(szDeviceID, sizeof(szDeviceID), "hw:%d,%d", card, dev);
            std::string tempString;
            tempString += szDeviceID;
            tempString += ' ';
            tempString += snd_ctl_card_info_get_name(info);
            strncpy(listOfDev[deviceCount].DevName, tempString.c_str(), 31);

            listOfDev[deviceCount].DevId = deviceCount;
            deviceCount++;
        }
        snd_ctl_close(handle);
    }
    return deviceCount;
}
void AudioReplayALSA::QueueToReplay(AudioFrame *frame)
{
    {
        AudioFrame temp(*frame);
        std::lock_guard<std::mutex> lk(_mut);

        _mapOfQueues[frame->authorPort].push(*frame);
        ++_framesInQueuesNumber;

        // TODO code below might fail
//        auto itr = _playbackQueue.begin();
//        for (itr = _playbackQueue.begin(); itr != _playbackQueue.end(); ++itr)
//        {
//            if (itr->authorPort != frame->authorPort)
//            {
//                *itr = mix_frames(*itr, *frame);
//                break;
//            }
//        }
//        if (itr == _playbackQueue.end())
//        {
//            _playbackQueue.push_back(*frame);
//        }
    }
    _cond.notify_one();
}
void AudioReplayALSA::StopReplay()
{
    _isPlaying = false;
    if (_playingThread.joinable())
    {
        _playingThread.join();
    }
}
void AudioReplayALSA::StartReplay()
{
    if (!_initialized)
    {
        printf("Tried to play before initializing.\n");
        return;
    }

    _playingThread = std::thread(&AudioReplayALSA::PlayingJob, this);
}

void AudioReplayALSA::PlayingJob() {

    if (_isPlaying)
    {
        return;
    }
    _isPlaying = true;

    while (_isPlaying)
    {
        int rc;
        std::unique_lock<std::mutex> lk (_mut);
        auto audioReady = _cond.wait_for(lk, std::chrono::seconds(10),[this](){return _framesInQueuesNumber > MIN_READY_FRAMES;});
        lk.unlock();

        if (!audioReady)
            continue;

        while(true)
        {
            std::queue<AudioFrame> mergeAndPlayQueue;
            lk.lock();
            for (auto itr = _mapOfQueues.begin(); itr != _mapOfQueues.end(); ++itr)
            {
               if (!itr->second.empty())
               {
                   mergeAndPlayQueue.push(itr->second.front());
                   itr->second.pop();
                   --_framesInQueuesNumber;
               }
            }
            lk.unlock();
            if (mergeAndPlayQueue.empty())
                break;

            while (mergeAndPlayQueue.size() > 1)
            {
                AudioFrame af;
                af = mergeAndPlayQueue.front();
                mergeAndPlayQueue.pop();
                mergeAndPlayQueue.front() = mix_frames(af, mergeAndPlayQueue.front());
            }

            AudioFrame frame(mergeAndPlayQueue.front());
            mergeAndPlayQueue.pop();

            while ((rc = snd_pcm_writei(_alsaHandle, frame.Data, _alsaFramesPerPeriod)) < 0)
            {
                if (rc == -EAGAIN)
                    continue;
                printf("Output buffer underrun (%d so far)\n", ++_alsaUnderrunsCount);
                snd_pcm_prepare(_alsaHandle);
            }
            if (rc != (int)_alsaFramesPerPeriod) {
                printf("short write, write %d frames\n", rc);
            }
        }
    }
}


void AudioReplayALSA::Cleanup()
{
    if (_alsaHandle != nullptr)
    {
        snd_pcm_drain(_alsaHandle);
        snd_pcm_close(_alsaHandle);
        _alsaHandle = nullptr;
    }
    if (_alsaBuffer != nullptr)
    {
        free(_alsaBuffer);
        _alsaBuffer = nullptr;
    }
}
void AudioReplayALSA::OpenDevice()
{
    const unsigned int devicesListSize = GetNrOfReplayDevs();
    auto devicesList = new AudioGrabbnigDev[devicesListSize];
    GetLstOfReplayDevs(devicesList, devicesListSize);

    std::string tempString("plug");
    tempString += devicesList[_deviceNumber].DevName;
    std::string tempSubStr = tempString.substr(0, 10);
    int rc = snd_pcm_open(&_alsaHandle, tempString.substr(0, 10).c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0)
    {
        if (devicesList != nullptr)
        {
            delete[] devicesList;
            devicesList = nullptr;
        }
        printf("ALSA ERROR: %s\n", snd_strerror(rc));
        throw std::runtime_error("Unable to open pcm device\n");
    }
    printf("Device %s opened successfully.\n", tempString.c_str());

    if (devicesList != nullptr)
    {
        delete[] devicesList;
        devicesList = nullptr;
    }
}
void AudioReplayALSA::FillParameters()
{
    int err;
    // Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&_alsaParams);
    // Fill it in with default values.
    snd_pcm_hw_params_any(_alsaHandle, _alsaParams);
    // Interleaved mode
    snd_pcm_hw_params_set_access(_alsaHandle, _alsaParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    switch (_signalParams.BitPerSample)
    {
        case 8:
            err = snd_pcm_hw_params_set_format(_alsaHandle, _alsaParams, SND_PCM_FORMAT_U8);
            break;
        case 16:
            err = snd_pcm_hw_params_set_format(_alsaHandle, _alsaParams, SND_PCM_FORMAT_S16_LE);
            break;
        case 24:
            err = snd_pcm_hw_params_set_format(_alsaHandle, _alsaParams, SND_PCM_FORMAT_S24_LE);
            break;
        case 32:
            err = snd_pcm_hw_params_set_format(_alsaHandle, _alsaParams, SND_PCM_FORMAT_S32_LE);
            break;
        default:
            printf("Bits per sample are set to incorrect value of: %d (8/16/24/32 supported)\n", _signalParams.BitPerSample);
            throw std::runtime_error("Cannot continue without valid BitPerSample value.");
            break;
    }
    if (err < 0)
    {
        printf("Sample (%d bit) format not available for recording: %s\n", _signalParams.BitPerSample, snd_strerror(err));
        throw std::runtime_error("Couldn't set proper BitPerSample format.");
    }
    err = snd_pcm_hw_params_set_channels(_alsaHandle, _alsaParams, _signalParams.NumberOfChannels);
    if (err < 0)
    {
        printf("That number of channels (%d) is not avalible: %s\n", _signalParams.NumberOfChannels, snd_strerror(err));
        throw std::runtime_error("Couldn't set proper NumberOfChannels.");

    }
    _alsaVal = _signalParams.SampleRate;
    snd_pcm_hw_params_set_rate_near(_alsaHandle, _alsaParams, &_alsaVal, &_alsaDir);

    if ((err = snd_pcm_hw_params_set_periods_near(_alsaHandle, _alsaParams, (unsigned int*)&_alsaPeriodsPerBuffer, 0)) < 0)
    {
        printf("Error setting # fragments to %d: %s\n", _alsaPeriodsPerBuffer,	snd_strerror(err));

    }
    // FramesPerPeriod are ALSA's version of SamplesPerFrame
    _alsaFrameSize = (_signalParams.BitPerSample / 8) * _signalParams.NumberOfChannels;
    _alsaFramesPerPeriod = _signalParams.SamplesPerFrame;
    _alsaFramesPerBuffer = _alsaFramesPerPeriod * _alsaPeriodsPerBuffer;
    _alsaPeriodSize = _alsaFramesPerBuffer * _alsaFrameSize / _alsaPeriodsPerBuffer;

    _alsaBufferSize = _alsaFramesPerBuffer * _alsaFrameSize;
    if ((err = snd_pcm_hw_params_set_buffer_size_near(_alsaHandle, _alsaParams, &_alsaFramesPerBuffer)) < 0)
    {
        printf("Error setting buffer_size %ld frames (%d bytes): %s\n", _alsaFramesPerBuffer, _alsaBufferSize, snd_strerror(err));
        throw std::runtime_error ("Cannot conitniue without proper buffer size.\n");
    }
    if (_alsaPeriodSize != (_alsaFramesPerBuffer * _alsaFrameSize) / _alsaPeriodsPerBuffer)
    {
        printf("Could not set requested period size, asked for %d got %ld\n", _alsaPeriodSize, (_alsaFramesPerBuffer * _alsaFrameSize) / _alsaPeriodsPerBuffer);
        _alsaPeriodSize = (_alsaFramesPerBuffer * _alsaFrameSize) / _alsaPeriodsPerBuffer;
    }
    // Write the parameters to the driver
    err = snd_pcm_hw_params(_alsaHandle, _alsaParams);
    if (err < 0)
    {
        printf("unable to set hw parameters: %s\n", snd_strerror(err));
        throw std::runtime_error("Cannot conitniue without ALSA parameters set.");
    }
}

AudioFrame AudioReplayALSA::mix_frames(AudioFrame &frame1, AudioFrame &frame2) {
    if (frame1.DataSize != frame2.DataSize)
        throw std::runtime_error("Cannot join frames of different sizes.\n");

    //file_record2.write((char*)frame1.Data, frame1.DataSize);
    //file_record3.write((char*)frame2.Data, frame2.DataSize);

    AudioFrame resultFrame(frame1.DataSize);
    int16_t* out_buffer = new int16_t[frame1.DataSize / 2];
    auto outBufferPtr = out_buffer;
    int16_t* ptr1 = reinterpret_cast<int16_t*>(frame1.Data);
    int16_t* ptr2 = reinterpret_cast<int16_t*>(frame2.Data);

    for (int i = 0; i < frame1.DataSize / 2; i++)
    {
        int16_t mixed_sample = mix_sample(*ptr1, *ptr2);
        *outBufferPtr = mixed_sample;
        ++outBufferPtr;
        ++ptr1;
        ++ptr2;
    }

    memcpy(resultFrame.Data, out_buffer, resultFrame.DataSize);

    delete [] out_buffer;

    return resultFrame;
}

int16_t AudioReplayALSA::mix_sample(int16_t sample1, int16_t sample2) {
    // suming values casted to int32_t to prevent overflow
    const int32_t result(static_cast<int32_t>(sample1) + static_cast<int32_t>(sample2));

    // clipping if sum exceeded max/min 16 bit int value
    typedef std::numeric_limits<int16_t> range;
    if (range::max() < result)
        return range::max();
    else if (range::min() > result)
        return range::min();
    else
        return static_cast<int16_t>(result);
}





