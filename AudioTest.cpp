//
// Created by michael on 18.04.19.
//


#include "AudioDecoderOpus.h"
#include "AudioEncoderOpus.h"
#include "AudioFrame.h"
#include "AudioGrabberALSA.h"
#include "AudioReplayALSA.h"
#include "ClientRTP.h"
#include "IAudioDecoded.h"
#include "IAudioEncoded.h"
#include "IAudioFrameProducer.h"
#include "IClientCallback.h"
#include <alsa/asoundlib.h>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <thread>


#define FRAMES_COUNT 960
// 44100 is supported by ALSA, but not by Opus
#define SAMPLE_RATE 48000
// use stereo for better results
#define CHANNELS_COUNT 2
// 16 and 32 works, 24 not supported, 8 is buggy (ALSA)
// 16 is optimal
#define BITS_PER_SAMPLE 16
#define TARGET_BITRATE 32

using namespace std::chrono_literals;

class AudioTest : IAudioFrameProducer, IAudioDecoded, IAudioEncoded, IClientCallback {

public:
    AudioTest() {
        int captureDeviceNr = 0;
        int playbackDeviceNr = 0;

        _replay = new AudioReplayALSA();
        _grabber = new AudioGrabberALSA();
        _encoder = new AudioEncoderOpus();
        _decoder = new AudioDecoderOpus();
        _client = new ClientRTP(this);


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

        _client->initialize();
        std::this_thread::sleep_for(2s);
        InitAudioReplay(_replay, playbackDeviceNr);
        InitAudioGrabber(_grabber, captureDeviceNr);
        InitAudioEncoder(_encoder);
        InitAudioDecoder(_decoder);

        // starting grabbing thread, record to file for few seconds, delete grabber to close test.raw file
        _replay->StartReplay();
        _grabber->StartGrabbing();

        std::this_thread::sleep_for(15s);
        _grabber->StopGrabbing();
        _grabber->UnInit();
        delete _grabber;
        _grabber = nullptr;

        // playing recorded audio from file
        _fileToReadDesc = fopen("test.raw", "rb");
        int bufferSize = FRAMES_COUNT * CHANNELS_COUNT * (BITS_PER_SAMPLE / 8);
        auto buffer = (unsigned char *) malloc(bufferSize);
        printf("Proceeding to play...\n");
        float bytesPlayed = 0;
        int bytesRead = 0;
        while ((bytesRead = fread(buffer, sizeof(char), bufferSize, _fileToReadDesc)) > 0) {
            bytesPlayed += bytesRead;
            if (bytesRead != bufferSize)
                std::cout << "Short read from file.\n";
            AudioFrame af;
            af.Data = buffer;
            af.DataSize = bufferSize;
            af.NumberOfSamples = FRAMES_COUNT;
            _replay->QueueToReplay(&af);
        }

        printf("Playing ended. Played %f bytes.\n", bytesPlayed);
        std::this_thread::sleep_for(5s);
        _replay->StopReplay();
        printf("Playing should really end here.\n");
        _client->uninit();
    }

    ~AudioTest() {
        if (_grabber != nullptr) {
            _grabber->StopGrabbing();
            _grabber->UnInit();
            delete _grabber;
            _grabber = nullptr;
        }

        if (_replay != nullptr) {
            _replay->UnInit();
            delete _replay;
            _replay = nullptr;
        }

        if (_encoder != nullptr) {
            _encoder->UnInit();
            delete _encoder;
            _encoder = nullptr;
        }

        if (_decoder != nullptr) {
            _decoder->UnInit();
            delete _decoder;
            _decoder = nullptr;
        }

        if (_client != nullptr)
        {
            delete _client;
            _client = nullptr;
        }

        if (_fileToReadDesc != nullptr) {
            fclose(_fileToReadDesc);
            _fileToReadDesc = nullptr;
        }
    }

private:
    AudioGrabberALSA *_grabber = nullptr;
    AudioReplayALSA *_replay = nullptr;
    AudioEncoderOpus *_encoder = nullptr;
    AudioDecoderOpus *_decoder = nullptr;
    ClientRTP* _client = nullptr;

    FILE *_fileToReadDesc;

    void InitAudioGrabber(AudioGrabberALSA *ag, int devNr) {
        ag->BeginInit();
        ag->SetParam(AudioGrabberALSA::InitParam_DeviceNumber_Int32, devNr);
        ag->SetParam(AudioGrabberALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        ag->SetParam(AudioGrabberALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        ag->SetParam(AudioGrabberALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        ag->SetParam(AudioGrabberALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        ag->SetParam(AudioGrabberALSA::InitParam_IAudioDataGrabbed, (void *) (IAudioFrameProducer *) this);
        ag->EndInit();
    }

    static void InitAudioReplay(AudioReplayALSA *ar, int devNr) {
        ar->BeginInit();
        ar->SetParam(AudioReplayALSA::InitParam_DeviceNumber_Int32, devNr);
        ar->SetParam(AudioReplayALSA::InitParam_Bitwidth_Int32, BITS_PER_SAMPLE);
        ar->SetParam(AudioReplayALSA::InitParam_Channels_Int32, CHANNELS_COUNT);
        ar->SetParam(AudioReplayALSA::InitParam_Samplerate_Int32, SAMPLE_RATE);
        ar->SetParam(AudioReplayALSA::InitParam_SamplesPerFrame_Int32, FRAMES_COUNT);
        ar->EndInit();
    }

    void InitAudioEncoder(AudioEncoderOpus *ae) {
        ae->BeginInit();
        ae->SetParam(AudioEncoderOpus::InitParam_IAudioEncoded, (void *) (IAudioEncoded *) this);
        ae->SetParam(AudioEncoderOpus::InitParam_Int32_NumberOfChannels, CHANNELS_COUNT);
        ae->SetParam(AudioEncoderOpus::InitParam_Int32_SampleRate, SAMPLE_RATE);
        ae->SetParam(AudioEncoderOpus::InitParam_Int32_BitPerSample, BITS_PER_SAMPLE);
        ae->SetParam(AudioEncoderOpus::InitParam_Int32_TargetBitrateInKbps, TARGET_BITRATE);
        ae->EndInit();
    }

    void InitAudioDecoder(AudioDecoderOpus *ad) {
        ad->BeginInit();
        ad->SetParam(AudioDecoderOpus::InitParam_IAudioDecoded, (void *) (IAudioDecoded *) this);
        ad->SetParam(AudioDecoderOpus::InitParam_Int32_NumberOfChannels, CHANNELS_COUNT);
        ad->SetParam(AudioDecoderOpus::InitParam_Int32_SampleRate, SAMPLE_RATE);
        ad->SetParam(AudioDecoderOpus::InitParam_Int32_BitPerSample, BITS_PER_SAMPLE);
        ad->EndInit();
    }


    void ListInDevs() {
        unsigned int cnt = _grabber->GetNrOfGrabbingDevs();
        printf("Audio dev cnt: %u\n", cnt);
        AudioGrabberALSA::TAudioGrabbnigDev devList[32];
        unsigned int c = _grabber->GetLstOfGrabbingDevs(devList);

        for (unsigned int i = 0; i < c; i++) {
            printf("%u -> %s\n", devList[i].nrDev, devList[i].DevName);
        }
    }

    void ListOutDevs() {
        unsigned int cnt = _replay->GetNrOfReplayDevs();
        printf("Audio out dev cnt: %u\n", cnt);
        AudioReplayALSA::AudioGrabbnigDev devList[32];
        unsigned int c = _replay->GetLstOfReplayDevs(devList, 32);
        for (unsigned int i = 0; i < c; i++) {
            printf("%u -> %s\n", devList[i].DevId, devList[i].DevName);
        }
    }

    void AudioFrameProducer_NewData(AudioFrame *frame) override
    {
        _encoder->Encode(frame);
    }

    void AudioEncoded(EncodedAudio *audioPacket) override
    {
        _client->sendData(audioPacket);
    }

    void AudioDecoded(AudioFrame *frame) override
    {
        _replay->QueueToReplay(frame);
    }

    void ClientCallback_MessageRecieved(EncodedAudio* audioPacket) override
    {
        _decoder->Decode(audioPacket);
    }
};

int main()
{
    AudioTest test;

    return 0;
}