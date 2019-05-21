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
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpsourcedata.h>
#include <jrtplib3/rtcpapppacket.h>

#include <queue>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <cstring>
#include <utility>
#include <set>
#include "sha256.h"

#include "EncodedAudio.h"

using namespace jrtplib;



void checkerror(int rtperr)
{
	if (rtperr < 0)
	{
		std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
		exit(-1);
	}
}

//
// The new class routine
//

class MyRTPSession : public RTPSession
{
protected:
	void OnBYEPacket(RTPSourceData *dat) override
	{
		if (dat->IsOwnSSRC())
			return;
		
		uint32_t ip;
		uint16_t port;
		
		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort()-1;
		}
		else
			return;
		
		RTPIPv4Address dest(ip,port);
		DeleteDestination(dest);
		
		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
	}

	void OnAPPPacket (RTCPAPPPacket *apppacket, const RTPTime &receivetime, const RTPAddress *senderaddress) override
	{

		
		std::cout << "APP packet!\n";
		uint32_t ip;
		uint16_t port;

		const RTPIPv4Address *addr = (const RTPIPv4Address *)(senderaddress);
		ip = addr->GetIP();
		port = addr->GetPort();

		if (verifiedAddresses.find(std::make_pair(ip, port-1)) != verifiedAddresses.end())
			return;

		size_t data_size = apppacket->GetAPPDataLength();
		char* data_buff = new char[data_size];
		std::string passFromClient ((const char*) apppacket->GetAPPData());
		passFromClient = passFromClient.substr(0, 64);
		
		//memcpy(data_buff, (const char*)apppacket->GetAPPData(), data_size);
		std::string password = "HASLO";
		std::string passwordHash = sha256(password);
		std::cout << "Password is: " << password << std::endl;
		
		std::cout << "Password hash from client: " << passFromClient << std::endl;
		std::cout << "Correct password hash: " << passwordHash << std::endl;

		if (passwordHash != passFromClient)
		{
			std::cout << "Wrong password!\n";
			AddToIgnoreList(*senderaddress);
		}
		else
		{
			RTPIPv4Address dest(ip,port-1);
			AddDestination(dest);
			verifiedAddresses.insert(std::make_pair(ip,port-1));
			std::cout << "Correct password. Destination added.\n";
		}
		
		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "APP Packet received from: " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;

		int accepted = 12;
		
		const uint8_t name[4] = {'A','B','C','D'};
		// TODO make some valid response from server to confirm
		int status = SendRTCPAPPPacket(0, name, (void*)&accepted, sizeof(int));
		checkerror(status);



		delete [] data_buff;
	}

	void OnRemoveSource(RTPSourceData *dat)
	{
		if (dat->IsOwnSSRC())
			return;
		if (dat->ReceivedBYE())
			return;
		
		uint32_t ip;
		uint16_t port;
		
		if (dat->GetRTPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort();
		}
		else if (dat->GetRTCPDataAddress() != 0)
		{
			const RTPIPv4Address *addr = (const RTPIPv4Address *)(dat->GetRTCPDataAddress());
			ip = addr->GetIP();
			port = addr->GetPort()-1;
		}
		else
			return;
		
		RTPIPv4Address dest(ip,port);
		DeleteDestination(dest);
		
		struct in_addr inaddr;
		inaddr.s_addr = htonl(ip);
		std::cout << "Deleting destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
	}

public:

	std::set<std::pair<int32_t, int16_t>> verifiedAddresses; 
};

int main(void)
{
#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
#endif // RTP_SOCKETTYPE_WINSOCK
	
	MyRTPSession sess;
	uint16_t portbase;
	std::string ipstr;
	int status,i,num;

        // First, we'll ask for the necessary information
		
	std::cout << "This version should handle APP packet.\n";

	std::cout << "Enter local portbase:" << std::endl;
	std::cin >> portbase;
	std::cout << std::endl;
	
	std::cout << std::endl;
	std::cout << "Number of seconds you wish to wait:" << std::endl;
	std::cin >> num;
	

	RTPUDPv4TransmissionParams transparams;
	RTPSessionParams sessparams;

	sessparams.SetOwnTimestampUnit(1.0/48000.0);		
	sessparams.SetAcceptOwnPackets(true);
	sessparams.SetReceiveMode(RTPTransmitter::ReceiveMode::IgnoreSome);

	transparams.SetPortbase(portbase);

	status = sess.Create(sessparams,&transparams);
	checkerror(status);

	std::queue<std::pair<EncodedAudio, RTPIPv4Address>> sendingQueue;

	RTPTime starttime = RTPTime::CurrentTime();
	bool done = false;

	while (!done)
	{


		if (!sendingQueue.empty())
		{
			sess.DeleteDestination(sendingQueue.front().second);
			auto infoPlusBuffer = new unsigned char[sendingQueue.front().first.DataSize+4];
			uint16_t portNr = sendingQueue.front().second.GetPort();
			memcpy(infoPlusBuffer, &portNr, 2);
			infoPlusBuffer += 2;
			memcpy(infoPlusBuffer, sendingQueue.front().first.Data, sendingQueue.front().first.DataSize);
			infoPlusBuffer -= 2;
			status = sess.SendPacket(infoPlusBuffer, sendingQueue.front().first.DataSize+2, 0, false, 960);
			delete [] infoPlusBuffer;
			checkerror(status);
			sess.AddDestination(sendingQueue.front().second);
			sendingQueue.pop();
		}

		sess.BeginDataAccess();

		if (sess.GotoFirstSourceWithData())
		{
			do
			{
				RTPPacket *pack;
				auto currentSourceAddress = (const RTPIPv4Address *)sess.GetCurrentSourceInfo()->GetRTPDataAddress();
				uint32_t currentSourceIP = currentSourceAddress->GetIP();
				uint16_t currentSourcePort = currentSourceAddress->GetPort();

				if (sess.verifiedAddresses.find(std::make_pair(currentSourceIP, currentSourcePort)) == sess.verifiedAddresses.end())
				{
					sess.AddToIgnoreList(*currentSourceAddress);
					std::cout << "Unverified source tried to send RTP packet. Adding to ignore list.\n";
					// important: deleting packet so that GotoNextSourceWithData doesnt get stuck on this source
					pack = sess.GetNextPacket();
					sess.DeletePacket(pack);
					continue;
				}

				while ((pack = sess.GetNextPacket()) != NULL)
				{
					// processing received data
			        EncodedAudio ea;
					ea.Data = pack->GetPayloadData();
					ea.DataSize = pack->GetPayloadLength();
                    sendingQueue.push(std::make_pair(ea, *currentSourceAddress));
					sess.DeletePacket(pack);
				}
			} while (sess.GotoNextSourceWithData());
		}
		
		sess.EndDataAccess();

#ifndef RTP_SUPPORT_THREAD
		status = sess.Poll();
		checkerror(status);
#endif // RTP_SUPPORT_THREAD
		
		RTPTime t = RTPTime::CurrentTime();
		t -= starttime;
		if (t > RTPTime(num))
			done = true;

		RTPTime::Wait(RTPTime(0.005));
	}
	
	sess.BYEDestroy(RTPTime(10,0),0,0);

	return 0;
}

