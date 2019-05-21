/*
   Here's a small IPv4 example: it asks for a portbase and a destination and 
   starts sending packets to that destination.
*/

#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
#include <jrtplib3/rtppacket.h>
#include <jrtplib3/rtcpapppacket.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <fstream>

#include "sha256.h"
#include "AudioFrame.h"
#include "EncodedAudio.h"
#include "AudioEncoderOpus.h"

using namespace jrtplib;

#define DEFAULT_IP "192.168.0.31"
#define DEFAULT_DEST_PORT 5000

#define FRAMES_COUNT 960
// 44100 is supported by ALSA, but not by Opus
#define SAMPLE_RATE 48000
// use stereo for better results
#define CHANNELS_COUNT 2
// 16 and 32 works, 24 not supported, 8 is buggy (ALSA)
// 16 is optimal
#define BITS_PER_SAMPLE 16
#define TARGET_BITRATE 32
//
// This function checks if there was a RTP error. If so, it displays an error
// message and exists.
//

class MyRTPClientSession : public RTPSession
{
	void OnAPPPacket (RTCPAPPPacket *apppacket, const RTPTime &receivetime, const RTPAddress *senderaddress) override
	{

		std::cout << "APP packet received!\n";
	}
};
void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

void InitAudioEncoder(AudioEncoderOpus *ae) {
	ae->BeginInit();
	ae->SetParam(AudioEncoderOpus::InitParam_IAudioEncoded, nullptr);
	ae->SetParam(AudioEncoderOpus::InitParam_Int32_NumberOfChannels, CHANNELS_COUNT);
	ae->SetParam(AudioEncoderOpus::InitParam_Int32_SampleRate, SAMPLE_RATE);
	ae->SetParam(AudioEncoderOpus::InitParam_Int32_BitPerSample, BITS_PER_SAMPLE);
	ae->SetParam(AudioEncoderOpus::InitParam_Int32_TargetBitrateInKbps, TARGET_BITRATE);
	ae->EndInit();
}
//
// The main routine
//

int main( int argc, char* argv[] )
{
	AudioEncoderOpus encoder;
	InitAudioEncoder(&encoder);

	if (argc < 3)
	{
		std::cout << "Need port number and sound file as arguments.\n";
		return -1;
	}

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
#endif // RTP_SOCKETTYPE_WINSOCK
	MyRTPClientSession sess;
	uint16_t portbase, destport;
	uint32_t destip;
	std::string ipstr(DEFAULT_IP);
	int status, i ,num;

	std::ifstream inFile;
	inFile.open(argv[2], std::ios_base::binary);
	size_t fileSize;
    inFile.seekg(0, std::ios::end);
    fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

	auto buffer = new char [fileSize];
	inFile.read(buffer, fileSize);

	portbase = atoi(argv[1]);

	std::cout << "Using version " << RTPLibraryVersion::GetVersion().GetVersionString() << std::endl;
		
	
	destip = inet_addr(ipstr.c_str());
	if (destip == INADDR_NONE)
	{
		std::cerr << "Bad IP address specified" << std::endl;
		return -1;
	}

	destip = ntohl(destip);
	
	destport = DEFAULT_DEST_PORT;
	
	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;
	
	sessparams.SetOwnTimestampUnit(1.0/48000.0);
	sessparams.SetReceiveMode(RTPTransmitter::ReceiveMode::AcceptSome);	
	
	sessparams.SetAcceptOwnPackets(true);
	transparams.SetPortbase(portbase);
	status = sess.Create(sessparams,&transparams);	
	checkerror(status);
	
	RTPIPv4Address addr(destip,destport);
	sess.AddToAcceptList(addr);
	RTPIPv4Address addr2(destip,destport+1);
	sess.AddToAcceptList(addr2);
	status = sess.AddDestination(addr);

	checkerror(status);

	uint8_t subtype = 0;
	const uint8_t name[4] = {'A','B','C','D'};
	size_t data_size = 16;
	std::string password = "HASLO";
	
	std::string pass_hash = sha256(password);
	// for (int i = 0; i < data_size; i++)
	// 	app_data[i] = i;
	std::cout << "Password is: " << password << std::endl;
	std::cout << "Password hash: " << pass_hash << std::endl;
	std::cout << "Password hash len: " << pass_hash.size() << std::endl;
	
	for (int i = 0; i < 1; i++)
	{
		status = sess.SendRTCPAPPPacket(subtype, name, (void*)pass_hash.c_str(), pass_hash.size());
		checkerror(status);
		std::cout << "APP sent.\n";
		
	}
	
	int periodSize = FRAMES_COUNT * CHANNELS_COUNT * ((BITS_PER_SAMPLE)/8);
	char* itrPtr = buffer;
	for (int i = 0; i < double(fileSize) / periodSize; i++)
	{
		AudioFrame af;
		af.Data = (unsigned char*) itrPtr;
		af.DataSize = periodSize;
		af.NumberOfSamples = FRAMES_COUNT;
		EncodedAudio ea = encoder.EncodeReturn(af);
		status = sess.SendPacket((void*) ea.Data, ea.DataSize, 0, false, 960);
		checkerror(status);
		RTPTime::Wait(RTPTime(0.018)); 
		itrPtr += periodSize;
	}
	
	sess.BYEDestroy(RTPTime(10,0),0,0);

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSACleanup();
#endif // RTP_SOCKETTYPE_WINSOCK
	return 0;
}
