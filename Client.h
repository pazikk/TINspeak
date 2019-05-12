//
// Created by michael on 11.05.19.
//

#ifndef CLION_CLIENT_H
#define CLION_CLIENT_H

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


#include "Log.h"

#define MAX_PACKET_SIZE 4000


class Client {
private:
    int sock;
    int len;
    struct sockaddr_in server;
    struct hostent *hp;
    char buf[1024];
    char data[MAX_PACKET_SIZE];
    IClientCallback* _msgRecieved = nullptr;
    std::thread recvt;
public:
    Client(IClientCallback* callback);
    ~Client();


public:
    void recvmg();
    void initialize();
    void uninit();
    void sendData(EncodedAudio* ea);
};


#endif //CLION_CLIENT_H
