//
// Created by michael on 11.05.19.
//

#include "Client.h"

void Client::recvmg()
{
    unsigned char msg[MAX_PACKET_SIZE];
    int len;

    CLIENT_INFO("receiving audio");
    while((len = recv(sock,msg,MAX_PACKET_SIZE,0)) > 0 && isReceiving) {
        EncodedAudio ea;
        ea.Data = msg;
        ea.DataSize = len;
        _msgRecieved->ClientCallback_MessageRecieved(&ea);
        memset(msg,'\0',sizeof(msg));
    }
}

void Client::initialize() {

    sock = socket( AF_INET, SOCK_STREAM, 0 );
    if (sock == -1) {
        CLIENT_ERROR("opening socket");
        return;
    }

    server.sin_family = AF_INET;
    hp = gethostbyname("localhost");


    if (hp == (struct hostent *) 0) {
        return;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("94.172.194.18");
    len = sizeof(struct sockaddr_in);

    server.sin_port = htons(9999);

    if (connect(sock, (struct sockaddr *) &server, sizeof server) == -1) {
        CLIENT_ERROR("connecting stream socket");
        return;
    }


    isReceiving = true;
    recvt = std::thread(&Client::recvmg, this);

}

void Client::uninit() {
    if (recvt.joinable())
    {
        isReceiving = false;
        recvt.join();
    }

}

Client::Client(IClientCallback* callback) {
    _msgRecieved = callback;
}

Client::~Client()
{
    CLIENT_INFO("shutting down");
    close(sock);
}

void Client::sendData(EncodedAudio *ea) {

    if (write(sock, ea->Data, ea->DataSize) == -1)
    {
        CLIENT_ERROR("Writing on stream socket");
        return;
    }

}


