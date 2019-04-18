//#include <chrono>
//#include <thread>
//#include <alsa/asoundlib.h>
//#include <iostream>
//#include <fstream>
//#include "AudioGrabberALSA.h"
//
//using namespace std::chrono_literals;
//
//class AudioGrabber
//{
//
//public:
//    AudioGrabber()
//    {
//
//        ListDevs();
//        _grabber = new AudioGrabberALSA();
//        InitAudioGrabber(_grabber);
//
//
//        pFile = fopen("notthisone.raw", "w");
//        _grabber->StartGrabbing();
//
//    }
//    ~AudioGrabber()
//    {
//
//        if (_grabber != nullptr)
//        {
//            _grabber->StopGrabbing();
//
//            _grabber->UnInit();
//            _grabber = nullptr;
//        }
//
//        if (pFile != nullptr)
//            fclose(pFile);
//    }
//
//private:
//    AudioGrabberALSA* _grabber = nullptr;
//    FILE * pFile;
//
//    void InitAudioGrabber(AudioGrabberALSA* ag)
//    {
//        ag->BeginInit();
//
//        ag->SetParam(AudioGrabberALSA::InitParam_DeviceNumber_Int32, 0);
//        ag->SetParam(AudioGrabberALSA::InitParam_Bitwidth_Int32, 16);
//        ag->SetParam(AudioGrabberALSA::InitParam_Channels_Int32, 1);
//        ag->SetParam(AudioGrabberALSA::InitParam_Samplerate_Int32, 48000);
//        ag->SetParam(AudioGrabberALSA::InitParam_SamplesPerFrame_Int32, 480);
//
//        ag->EndInit();
//    }
//
//    void ListDevs()
//    {
//        unsigned int cnt = _grabber->GetNrOfGrabbingDevs();
//        printf("Audio dev cnt: %u\n", cnt);
//        AudioGrabberALSA::TAudioGrabbnigDev devList[32];
//        unsigned int c = _grabber->GetLstOfGrabbnigDevs(devList, 32);
//        for (unsigned int i = 0; i < c; i++)
//        {
//            printf("%u -> %s\n", devList[i].nrDev, devList[i].DevName);
//        }
//
//    }
//};
//
//int main()
//{
//    AudioGrabber ag;
//    std::this_thread::sleep_for(5s);
//    return 0;
//}
