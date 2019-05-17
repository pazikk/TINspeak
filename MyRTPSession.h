//
// Created by michael on 17.05.19.
//

#ifndef CLION_MYRTPSESSION_H
#define CLION_MYRTPSESSION_H

#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpudpv4transmitter.h>
#include <jrtplib3/rtpipv4address.h>
#include <jrtplib3/rtpsessionparams.h>
#include <jrtplib3/rtperrors.h>
#include <jrtplib3/rtplibraryversion.h>
#include <jrtplib3/rtppacket.h>
#include <jrtplib3/rtpsession.h>
#include <jrtplib3/rtpsourcedata.h>
#include <iostream>


using namespace jrtplib;

class MyRTPSession : public RTPSession
{
protected:
    void OnNewSource(RTPSourceData *dat)
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
        AddDestination(dest);

        struct in_addr inaddr;
        inaddr.s_addr = htonl(ip);
        std::cout << "Adding destination " << std::string(inet_ntoa(inaddr)) << ":" << port << std::endl;
    }

    void OnBYEPacket(RTPSourceData *dat)
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
};


#endif //CLION_MYRTPSESSION_H
