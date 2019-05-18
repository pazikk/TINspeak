//
// Created by michael on 11.05.19.
//

#include "ClientRTP.h"
using namespace jrtplib;

void ClientRTP::recvmg()
{

    RTPTime starttime = RTPTime::CurrentTime();

    while (!done) {



        sess.BeginDataAccess();

        // check incoming packets
        if (sess.GotoFirstSourceWithData()) {
            do {
                RTPPacket *pack;

                while ((pack = sess.GetNextPacket()) != NULL) {
//                    if (pack->GetSSRC() == sess.GetLocalSSRC())
//                        continue;
                    // You can examine the data here
                    EncodedAudio ea;
                    ea.Data = (unsigned char*)pack->GetPayloadData();
                    // TODO analyze if data form packet wont be deleted
                    // before decoding or if you shouldn't discard some data here
                    // so that decoder won't get bad data

                    ea.DataSize = pack->GetPayloadLength();

                    _msgRecieved->ClientCallback_MessageRecieved(&ea);


                    // we don't longer need the packet, so
                    // we'll delete it
                    sess.DeletePacket(pack);
                }
            } while (sess.GotoNextSourceWithData());
        }

        sess.EndDataAccess();

        RTPTime t = RTPTime::CurrentTime();
        t -= starttime;
        if (t > RTPTime(60.0))
            done = true;

        RTPTime::Wait(RTPTime(0.005)); // TODO 0.020?
    }

    sess.BYEDestroy(RTPTime(10,0),0,0);
//    unsigned char msg[MAX_PACKET_SIZE];
//    int len;
//
//    while((len = recv(sock,msg,MAX_PACKET_SIZE,0)) > 0)
//    {
//        EncodedAudio ea;
//        ea.Data = msg;
//        ea.DataSize = len;
//        _msgRecieved->ClientCallback_MessageRecieved(&ea);
//        memset(msg,'\0',sizeof(msg));
//    }
}

void ClientRTP::initialize()
{

    int status, num;

    std::cout << "Using version " << RTPLibraryVersion::GetVersion().GetVersionString() << std::endl;

    // First, we'll ask for the necessary information

    std::cout << "Enter local portbase:" << std::endl;
    std::cin >> portbase;
    std::cout << std::endl;

    std::cout << "Enter the destination IP address" << std::endl;
    std::cin >> ipstr;
    destip = inet_addr(ipstr.c_str());
    if (destip == INADDR_NONE)
    {
        std::cerr << "Bad IP address specified" << std::endl;
        throw std::runtime_error("Bad IP address specified.");
    }

    // The inet_addr function returns a value in network byte order, but
    // we need the IP address in host byte order, so we use a call to
    // ntohl
    destip = ntohl(destip);

    std::cout << "Enter the destination port" << std::endl;
    std::cin >> destport;

    std::cout << std::endl;
    std::cout << "Number of packets you wish to be sent:" << std::endl;
    std::cin >> num;

    // Now, we'll create a RTP session, set the destination, send some
    // packets and poll for incoming data.

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    // IMPORTANT: The local timestamp unit MUST be set, otherwise
    //            RTCP Sender Report info will be calculated wrong
    // In this case, we'll be sending 10 samples each second, so we'll
    // put the timestamp unit to (1.0/10.0)
    sessparams.SetOwnTimestampUnit(1.0/48000.0); // TODO change from literal

    sessparams.SetAcceptOwnPackets(false);
    transparams.SetPortbase(portbase);
    status = sess.Create(sessparams, &transparams);
    checkerror(status);

    RTPIPv4Address addr(destip,destport);

    status = sess.AddDestination(addr);
    checkerror(status);

    recvt = std::thread(&ClientRTP::recvmg, this);
}

void ClientRTP::uninit()
{
    done = true;
    if (recvt.joinable())
    {
        recvt.join();
    }
}

ClientRTP::ClientRTP(IClientCallback* callback)
{
    _msgRecieved = callback;
}

ClientRTP::~ClientRTP()
{
    uninit();
}

void ClientRTP::sendData(EncodedAudio *ea)
{
    sess.SendPacket((void*)ea->Data, ea->DataSize, 0, false, 960); // TODO 960 should be calculated not hard coded
}


