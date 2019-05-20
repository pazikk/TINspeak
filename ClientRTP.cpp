//
// Created by michael on 11.05.19.
//

#include "ClientRTP.h"
#include "sha256.h"


using namespace jrtplib;

void ClientRTP::recvmg()
{

    RTPTime starttime = RTPTime::CurrentTime();

    while (!done) {



        _session.BeginDataAccess();

        // check incoming packets
        if (_session.GotoFirstSourceWithData()) {
            do {
                RTPPacket *pack;

                auto currentSourceAddress = (const RTPIPv4Address *)_session.GetCurrentSourceInfo()->GetRTPDataAddress();
                if (currentSourceAddress->GetPort() != _serverPort)
                {
                    std::cout << "Trying to decode this packet would cause an error.\n";
                    pack = _session.GetNextPacket();
                    _session.DeletePacket(pack);
                    continue;
                }

                while ((pack = _session.GetNextPacket()) != NULL) {
//                    if (pack->GetSSRC() == _session.GetLocalSSRC())
//                        continue;
                    // You can examine the data here
                    EncodedAudio ea;
                    ea.authorPort = *((uint16_t*)pack->GetPayloadData());
                    ea.Data = (unsigned char*)pack->GetPayloadData();
                    ea.Data += 2; //moving pointer past port info
                    // TODO analyze if data form packet wont be deleted
                    // before decoding or if you shouldn't discard some data here
                    // so that decoder won't get bad data

                    ea.DataSize = pack->GetPayloadLength();

                    _msgRecieved->ClientCallback_MessageRecieved(&ea);


                    // we don't longer need the packet, so
                    // we'll delete it
                    _session.DeletePacket(pack);
                }
            } while (_session.GotoNextSourceWithData());
        }

        _session.EndDataAccess();

        RTPTime t = RTPTime::CurrentTime();
        t -= starttime;
        if (t > RTPTime(60.0))
            done = true;

        RTPTime::Wait(RTPTime(0.005)); // TODO 0.020?
    }

    _session.BYEDestroy(RTPTime(10,0),0,0);
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

//    // First, we'll ask for the necessary information
//
//    std::cout << "Enter local _clientPort:" << std::endl;
//    std::cin >> _clientPort;
//    std::cout << std::endl;
//    if (_clientPort == 0)
//        _clientPort = DEFAULT_BASE_PORT;
//
//    std::cout << "Enter the destination IP address" << std::endl;
//    std::cin >> _serverName;
//    if (_serverName.size() == 1)
//        _serverName = std::string(DEFAULT_DEST_IP);
//    _serverIP = inet_addr(_serverName.c_str());
//
//    if (_serverIP == INADDR_NONE)
//    {
//        std::cerr << "Bad IP address specified" << std::endl;
//        throw std::runtime_error("Bad IP address specified.");
//    }

    // The inet_addr function returns a value in network byte order, but
    // we need the IP address in host byte order, so we use a call to
    // ntohl
//    _serverIP = ntohl(_serverIP);
//
//    std::cout << "Enter the destination port" << std::endl;
//    std::cin >> _serverPort;
//    if (_serverPort == 0)
//        _serverPort = DEFAULT_DEST_PORT;

    std::cout << std::endl;
//    std::cout << "Number of packets you wish to be sent:" << std::endl;
//    std::cin >> num;

    // Now, we'll create a RTP session, set the destination, send some
    // packets and poll for incoming data.

    RTPUDPv4TransmissionParams transparams;
    RTPSessionParams sessparams;

    // IMPORTANT: The local timestamp unit MUST be set, otherwise
    //            RTCP Sender Report info will be calculated wrong
    // In this case, we'll be sending 10 samples each second, so we'll
    // put the timestamp unit to (1.0/10.0)
    sessparams.SetOwnTimestampUnit(1.0/48000.0); // TODO change from literal
    sessparams.SetReceiveMode(RTPTransmitter::ReceiveMode::AcceptSome);

    sessparams.SetAcceptOwnPackets(true); // TODO false
    transparams.SetPortbase(_clientPort);
    status = _session.Create(sessparams, &transparams);
    checkerror(status);





    std::cout << "Pass hash: " << pass_hash << std::endl;
        // TODO lines below should repeat few times wating for server response
        status = _session.SendRTCPAPPPacket(subtype, name, (void*)pass_hash.c_str(), pass_hash.length());
        checkerror(status);
        std::cout << "APP sent.\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));

    _receiveThread = std::thread(&ClientRTP::recvmg, this);
}

void ClientRTP::uninit()
{
    done = true;
    if (_receiveThread.joinable())
    {
        _receiveThread.join();
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
    _session.SendPacket((void*)ea->Data, ea->DataSize, 0, false, 960); // TODO 960 should be calculated not hard coded
}

void ClientRTP::BeginInit() {
    if (_initialized)
        throw std::runtime_error("ClientRTP module is already initialized");
    if (_initInProgress)
        throw std::runtime_error("CilentRTP module initialization is already in progress");
    _initInProgress = true;
    printf("ClientRTP initialization started. Using JRTP version: %s\n",
            RTPLibraryVersion::GetVersion().GetVersionString().c_str());
}

void ClientRTP::SetParam(int param, int value) {
    switch ((InitParam)param)
    {
        case InitParam_Int16_ServerPort:
            _serverPort = static_cast<uint16_t>(value);
            printf("SetParam: ServerPort = %d", _serverPort);
            break;
        case InitParam_Int32_SampleRate:
            _sampleRate = value;
            printf("SetParam: ServerPort = %d", _sampleRate);
            break;
        default:
            printf("SetParam(int): UnknownParam = %d", value);
    }
}

void ClientRTP::SetParam(int param, void *value) {
    switch ((InitParam)param)
    {
        case InitParam_Communication_Callback:
            _msgRecieved = (IClientCallback*)value;
            printf("SetParam: AudioEncoded  Callback = 0x%X", _msgRecieved);
            break;
        case InitParam__ServerName:
            _serverName = (char *) value;
            printf("SetParam: ServerName = 0x%s", _serverName.c_str());
            break;

        default:
            printf("SetParam(*void): UnknownParam = 0x%X.", value);
    }
}

void ClientRTP::EndInit() {
    if (!_initInProgress) throw std::runtime_error("BeginInit needs to be called first. (BeforeEndInit).\n");
    _initInProgress = false;

    int err;
    bool initParamsError = false;

    if (_msgRecieved == nullptr)
    {
        printf("Communication callback interface must be set.\n");
        initParamsError = true;
    }
    if (_serverName.length() < 7)
    {
        printf("Invalid server name (%s).", _serverName.c_str());
        initParamsError = true;
    }
    if (_clientPort < 0 || _clientPort > MAX_PORT)
    {
        printf("Invalid client port (%d).  Supported range (1-65535).\n", _clientPort);
        initParamsError = true;
    }
    if (_serverPort < 0 || _serverPort > MAX_PORT)
    {
        printf("Invalid client port (%d).  Supported range (1-65535).\n", _serverPort);
        initParamsError = true;
    }

    _serverIP = inet_addr(_serverName.c_str());

    if (_serverIP == INADDR_NONE)
    {
        printf("Bad IP address specified\n");
        initParamsError = true;
    }

    _serverIP = ntohl(_serverIP);

    RTPUDPv4TransmissionParams transmissionParams;
    RTPSessionParams sessionParams;

    if (_sampleRate != 48000)
    {
        printf("Only 48000 hz sample rate is currently supported.\n");
        initParamsError = true;
    }

    sessionParams.SetOwnTimestampUnit(1.0/_sampleRate);
    sessionParams.SetReceiveMode(RTPTransmitter::ReceiveMode::AcceptSome);
    sessionParams.SetAcceptOwnPackets(false); // TODO false
    transmissionParams.SetPortbase(_clientPort);

    err = _session.Create(sessionParams, &transmissionParams);
    checkerror(err);

    // Adding server IP and RTP/RTCP ports to accept packets from
    RTPIPv4Address addr(_serverIP,_serverPort);
    _session.AddToAcceptList(addr);
    RTPIPv4Address addr2(_serverIP,_serverPort+1);
    _session.AddToAcceptList(addr2);

    err = _session.AddDestination(addr);
    checkerror(err);

    if (initParamsError) throw std::runtime_error("Couldn't initialize ClientRTP, bad params.\n");
    _initialized = true;

}

void ClientRTP::Start() {
    if (!_initialized)
        throw std::runtime_error("Cannot start uninitialized ClientRTP.\n");





}

void ClientRTP::Stop() {

}

void ClientRTP::UnInit() {
    Stop();
    _initialized = false;
}

void ClientRTP::Connect() {
    uint8_t subtype = 0;
    const uint8_t name[4] = {'A','B','C','D'};
    std::string password = "HASLO";
    std::string pass_hash = sha256(password);
    std::cout << "Password hash equals: " << pass_hash << std::endl;
    int status = _session.SendRTCPAPPPacket(subtype, name, (void*)pass_hash.c_str(), pass_hash.length());
    checkerror(status);
    std::cout << "APP sent.\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

}


