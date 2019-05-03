//
// Created by michael on 3/31/19.
//

#include "AudioGrabberALSA.h"
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <string>
#include <exception>

#define ALSA_PCM_NEW_HW_PARAMS_API

AudioGrabberALSA::AudioGrabberALSA()
{
    _initASignalParams.BitPerSample = 16;
    _initASignalParams.SampleRate = 48000;
    _initASignalParams.NumberOfChannels = 2;
    _initASignalParams.SamplesPerFrame = _initASignalParams.SampleRate / 25; //40 ms;

    _alsaBuffer = nullptr;
    _alsaBufferSize = 0;
    _alsaDir = 0;
    _alsaHandle = nullptr;
    _alsaParams = nullptr;
    _alsaFramesPerPeriod = 0;
    _alsaFramesPerBuffer = 0;
    _alsaPeriodSize = 0;
    _alsaFrameSize = 0;
    _alsaVal = 0;
    _alsaPeriodsPerBuffer = 2;
    _alsaOverrunsCount = 0;
}
AudioGrabberALSA::~AudioGrabberALSA()
{
    UnInit();
}
void AudioGrabberALSA::BeginInit()
{
    if (_initialized) throw std::runtime_error("AudioGrabber module is already initialized.");
    if (_initInProgress) throw std::runtime_error("AudioGrabber module initialization is already in progress.");
    _initInProgress = true;
    printf( "ALSA library version: %s\n", SND_LIB_VERSION_STR);
}
void AudioGrabberALSA::SetParam(int param, int value)
{
    switch ((InitParam)param)
    {
        case InitParam_Bitwidth_Int32:
            _initASignalParams.BitPerSample = value;
            printf( "SetParam: BitPerSample = %d.", _initASignalParams.BitPerSample);
            break;
        case InitParam_Samplerate_Int32:
            _initASignalParams.SampleRate = value;
            printf( "SetParam: Samplerate = %d.", _initASignalParams.SampleRate);
            break;
        case InitParam_Channels_Int32:
            _initASignalParams.NumberOfChannels = value;
            printf( "SetParam: NumberOfChannels = %d.", _initASignalParams.NumberOfChannels);
            break;
        case InitParam_DeviceNumber_Int32:
            _deviceNumber = value;
            printf( "SetParam: _deviceNumber = %d.", _deviceNumber);
            break;
        case InitParam_SamplesPerFrame_Int32:
            _initASignalParams.SamplesPerFrame = value;
            printf( "SetParam: SamplesPerFrame = %d.", _initASignalParams.SamplesPerFrame);
            break;
        default:
            printf( "SetParam: UnknownParam = %d", value);
    }
}

void AudioGrabberALSA::SetParam(int param, void * value)
{
    switch ((InitParam)param)
    {
        case InitParam_IAudioDataGrabbed:
            _audioGrabbed = (IAudioFrameProducer*)value;
            printf("SetParam: AudioDataGrabbed Callback = %X.", _audioGrabbed);
            break;
        default:
            printf("SetParam: UnknownParam = %X.", value);
    }
}
void AudioGrabberALSA::EndInit()
{
    if (!_initInProgress) throw std::runtime_error("Module BeginInit was not called");

    OpenDevice();
    FillParameters();

    _alsaFrameSize = (_initASignalParams.BitPerSample / 8) * _initASignalParams.NumberOfChannels;
    _alsaBufferSize = _alsaPeriodSize * _alsaPeriodsPerBuffer;
    _alsaBuffer = (char *)malloc(_alsaPeriodSize);

    _initialized = true;
    _initInProgress = false;
}
void AudioGrabberALSA::UnInit()
{
    StopGrabbing();
    Cleanup();
    _initialized = false;
}
void AudioGrabberALSA::Release()
{
    delete this;
}
unsigned int AudioGrabberALSA::GetNrOfGrabbingDevs()
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
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
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
unsigned int AudioGrabberALSA::GetLstOfGrabbingDevs(TAudioGrabbnigDev *devList)
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
            snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_CAPTURE);
            if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0)
            {
                continue;
            }
            char szDeviceID[20];
            snprintf(szDeviceID, sizeof(szDeviceID), "hw:%d,%d", card, dev);
            std::string tempString;
            tempString += szDeviceID;
            tempString += ' ';
            tempString += snd_ctl_card_info_get_name(info);
            strncpy(devList[deviceCount].DevName, tempString.c_str(), 31);
            devList[deviceCount].nrDev = deviceCount;
            deviceCount++;
        }
        snd_ctl_close(handle);
    }
    return deviceCount;
}
void AudioGrabberALSA::StartGrabbing()
{
    if (_isRecording)
    {
        return;
    }
    _isRecording = true;
    _recordingThread = std::thread(&AudioGrabberALSA::RecordingJob, this);
}
void AudioGrabberALSA::StopGrabbing()
{
    if (!_isRecording)
    {
        return;
    }
    _isRecording = false;
    if (_recordingThread.joinable())
    {
        _recordingThread.join();
    }
}
void AudioGrabberALSA::RecordingJob()
{
    FILE * test = fopen("test.raw","wb");
    bool restarting = true;
    int rc;
    printf( "\nAUDIO GRABBING started\n");
    while (_isRecording)
    {
        if (restarting)
        {
            restarting = false;
            snd_pcm_drop(_alsaHandle);
            snd_pcm_prepare(_alsaHandle);

            AudioFrame empty_frame;
            empty_frame.Data = nullptr;
            empty_frame.DataSize = 0;
            empty_frame.NumberOfSamples = 0;
            _audioGrabbed->AudioFrameProducer_NewData(&empty_frame);

        }
        while ((rc = snd_pcm_readi(_alsaHandle, _alsaBuffer, _alsaFramesPerPeriod)) < 0)
        {
            if (rc == -EAGAIN)
                continue;
            printf( "Input buffer overrun (%d so far)\n", ++_alsaOverrunsCount);
            restarting = true;
            snd_pcm_prepare(_alsaHandle);
        }
        if (rc != (int)_alsaFramesPerPeriod)
        {
            printf("Short read, read %d frames\n", rc);
            continue;
        }

        AudioFrame af;
        af.Data = (unsigned char*)_alsaBuffer;
        af.DataSize = _alsaPeriodSize;
        af.NumberOfSamples = _initASignalParams.SamplesPerFrame;
        fwrite(af.Data, sizeof(char), af.DataSize, test);
        _audioGrabbed->AudioFrameProducer_NewData(&af);
    }
    fclose(test);
    printf( "\nAUDIO GRABBING STOPPED\n");
}
void AudioGrabberALSA::FillParameters()
{
    int err;
    // Allocate a hardware parameters object.
    snd_pcm_hw_params_alloca(&_alsaParams);
    // Fill it in with default values.
    snd_pcm_hw_params_any(_alsaHandle, _alsaParams);
    // Interleaved mode
    snd_pcm_hw_params_set_access(_alsaHandle, _alsaParams, SND_PCM_ACCESS_RW_INTERLEAVED);
    switch (_initASignalParams.BitPerSample)
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
            printf(
                    "Bits per sample are set to incorrect value of: %d (8/16/24/32 supported)\n", _initASignalParams.BitPerSample);
            throw std::runtime_error( "Cannot continue without valid BitPerSample value.");
            break;
    }
    if (err < 0)
    {
        printf("Sample (%d bit) format not available for recording: %s\n", _initASignalParams.BitPerSample, snd_strerror(err));
        throw std::runtime_error("Couldn't set proper BitPerSample format.");
    }
    err = snd_pcm_hw_params_set_channels(_alsaHandle, _alsaParams, _initASignalParams.NumberOfChannels);
    if (err < 0)
    {
        printf("That number of channels (%d) is not avalible: %s\n", _initASignalParams.NumberOfChannels, snd_strerror(err));
        throw std::runtime_error("Couldn't set proper NumberOfChannels.");
    }
    _alsaVal = _initASignalParams.SampleRate;
    snd_pcm_hw_params_set_rate_near(_alsaHandle, _alsaParams, &_alsaVal, &_alsaDir);
    if ((err = snd_pcm_hw_params_set_periods_near(_alsaHandle, _alsaParams, (unsigned int*)&_alsaPeriodsPerBuffer, 0)) < 0)
    {
        printf("Error setting # fragments to %d: %s\n", _alsaPeriodsPerBuffer, snd_strerror(err));
    }
    // FramesPerPeriod are ALSA's version of SamplesPerFrame
    _alsaFrameSize = (_initASignalParams.BitPerSample / 8) * _initASignalParams.NumberOfChannels;
    _alsaFramesPerPeriod = _initASignalParams.SamplesPerFrame;
    _alsaFramesPerBuffer = _alsaFramesPerPeriod * _alsaPeriodsPerBuffer;
    _alsaPeriodSize = _alsaFramesPerBuffer * _alsaFrameSize / _alsaPeriodsPerBuffer;

    if ((err = snd_pcm_hw_params_set_buffer_size_near(_alsaHandle, _alsaParams, &_alsaFramesPerBuffer)) < 0)
    {
        printf("Error setting buffer_size %d frames: %s\n", _alsaFramesPerBuffer, snd_strerror(err));
        std::runtime_error("Cannot conitniue without proper buffer size.\n");
    }
    if (_alsaPeriodSize != _alsaFramesPerBuffer * _alsaFrameSize / _alsaPeriodsPerBuffer)
    {
        printf("Could not set requested buffer size, asked for %d got %d\n", _alsaPeriodSize, _alsaFramesPerBuffer * _alsaFrameSize / _alsaPeriodsPerBuffer);
        _alsaPeriodSize = _alsaFramesPerBuffer * _alsaFrameSize / _alsaPeriodsPerBuffer;
    }
    // Write the parameters to the driver
    err = snd_pcm_hw_params(_alsaHandle, _alsaParams);
    if (err < 0)
    {
        printf("unable to set hw parameters: %s\n", snd_strerror(err));
        throw std::runtime_error("Cannot conitniue without ALSA parameters set.");
    }
}
void AudioGrabberALSA::Cleanup()
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
void AudioGrabberALSA::OpenDevice()
{
    const unsigned int devicesListSize = GetNrOfGrabbingDevs();
    auto devicesList = new TAudioGrabbnigDev[devicesListSize];
    GetLstOfGrabbingDevs(devicesList);

    std::string tempString("plug");
    tempString += devicesList[_deviceNumber].DevName;
    std::string tempSubStr = tempString.substr(0, 10);
    int rc = snd_pcm_open(&_alsaHandle, tempSubStr.c_str(), SND_PCM_STREAM_CAPTURE, 0);
    if (rc < 0)
    {
        if (devicesList != nullptr)
        {
            delete[] devicesList;
            devicesList = nullptr;
        }
        printf("ALSA ERROR: %s\n", snd_strerror(rc));
        throw std::runtime_error("unable to open pcm device\n");
    }
    printf( "Device %s opened successfully.\n", tempString.c_str());

    if (devicesList != nullptr)
    {
        delete[] devicesList;
        devicesList = nullptr;
    }
}

