//
// Created by michael on 11.05.19.
//

#ifndef CLION_ICLIENTCALLBACK_H
#define CLION_ICLIENTCALLBACK_H
#include "EncodedAudio.h"

class IClientCallback {
public:
    virtual void ClientCallback_MessageRecieved(EncodedAudio* audioPacket) = 0;
};

#endif //CLION_ICLIENTCALLBACK_H
