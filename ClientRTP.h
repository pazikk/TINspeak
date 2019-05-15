//
// Created by michael on 11.05.19.
//

#ifndef CLION_CLIENTRTP_H
#define CLION_CLIENTRTP_H

#include <algorithm>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "IClientCallback.h"
#include "EncodedAudio.h"
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
#include <jrtplib3/rtppacket.h>
#include <iostream>

#define MAX_PACKET_SIZE 4000

using namespace jrtplib; // TODO bad practice

class ClientRTP {
private:
    jrtplib::RTPSession sess;
    uint16_t portbase,destport;
    uint32_t destip;
    std::string ipstr;

    volatile bool done = false;

    char data[MAX_PACKET_SIZE];
    IClientCallback* _msgRecieved = nullptr;
    std::thread recvt;
public:
    ClientRTP(IClientCallback* callback);
    ~ClientRTP();


public:
    // TODO move definition, rename
    void checkerror(int rtperr)
    {
        if (rtperr < 0)
        {
            std::cout << "ERROR: " << RTPGetErrorString(rtperr) << std::endl;
            exit(-1);
        }
    }
    void recvmg();
    void initialize();
    void uninit();
    void sendData(EncodedAudio* ea);
};


#endif //CLION_CLIENTRTP_H
