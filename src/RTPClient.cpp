//
// Created by michael on 11.05.19.
//

#include "RTPClient.h"
#include "sha256.h"


using namespace jrtplib;

void RTPClient::CommunicationThreadEntry()
{

   // RTPTime starttime = RTPTime::CurrentTime();

    while (!_done)
    {
        // locking mutex on jrtp internal packet queue
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
                    EncodedAudio ea;
                    // TODO change information about author from Port to SSRC (pack->GetSSRC())
                    ea.authorPort = *((uint16_t*)pack->GetPayloadData());
                    ea.Data = (unsigned char*)pack->GetPayloadData();
                    ea.Data += 2; //moving pointer past port info
                    ea.DataSize = pack->GetPayloadLength();

                    _msgRecieved->ClientCallback_MessageRecieved(&ea);
                    _session.DeletePacket(pack);
                }
            } while (_session.GotoNextSourceWithData());
        }
        _session.EndDataAccess();

//        RTPTime t = RTPTime::CurrentTime();
//        t -= starttime;
//        if (t > RTPTime(60.0))
//            _done = true;

        RTPTime::Wait(RTPTime(0.005)); // TODO 0.020?
    }

    _session.BYEDestroy(RTPTime(10,0),0,0);
}


RTPClient::RTPClient()
{
}

RTPClient::~RTPClient()
{
    UnInit();
}

void RTPClient::sendData(EncodedAudio *ea)
{
    _session.SendPacket((void*)ea->Data, ea->DataSize, 0, false, 960); // TODO 960 should be calculated not hard coded
}

void RTPClient::BeginInit() {
    if (_initialized)
        throw std::runtime_error("RTPClient module is already initialized");
    if (_initInProgress)
        throw std::runtime_error("CilentRTP module initialization is already in progress");
    _initInProgress = true;
    printf("RTPClient initialization started. Using JRTP version: %s\n",
            RTPLibraryVersion::GetVersion().GetVersionString().c_str());
}

void RTPClient::SetParam(int param, int value) {
    switch ((InitParam)param)
    {
        case InitParam_Int16_ClientPort:
            _clientPort = static_cast<uint16_t>(value);
            printf("SetParam: ClientPort = %d", _clientPort);
            break;
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

void RTPClient::SetParam(int param, void *value) {
    switch ((InitParam)param)
    {
        case InitParam_Communication_Callback:
            _msgRecieved = (IClientCallback*)value;
            printf("SetParam: AudioEncoded  Callback = 0x%X", _msgRecieved);
            break;
        case InitParam_ServerName:
            _serverName = (char *) value;
            printf("SetParam: ServerName = 0x%s", _serverName.c_str());
            break;

        default:
            printf("SetParam(*void): UnknownParam = 0x%X.", value);
    }
}

void RTPClient::EndInit() {
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
    if (_clientPort <= 0 || _clientPort > MAX_PORT)
    {
        printf("Invalid client port (%d).  Supported range (1-65535).\n", _clientPort);
        initParamsError = true;
    }
    if (_serverPort <= 0 || _serverPort > MAX_PORT)
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
    sessionParams.SetAcceptOwnPackets(false); // true?
    transmissionParams.SetPortbase(_clientPort);

    err = _session.Create(sessionParams, &transmissionParams);
    CheckRTPError(err);

    // Adding server IP and RTP/RTCP ports to accept packets from
    RTPIPv4Address addr(_serverIP,_serverPort);
    _session.AddToAcceptList(addr);
    RTPIPv4Address addr2(_serverIP,_serverPort+1);
    _session.AddToAcceptList(addr2);

    // Adding server address as destination
    err = _session.AddDestination(addr);
    CheckRTPError(err);

    if (initParamsError) throw std::runtime_error("Couldn't initialize RTPClient, bad params.\n");
    _initialized = true;

}

void RTPClient::Start() {
    if (!_initialized)
        throw std::runtime_error("Cannot start uninitialized RTPClient.\n");
    if (!_connected)
        throw std::runtime_error("You have to connect first.\n");

    _communicationThread = std::thread(&RTPClient::CommunicationThreadEntry, this);
}

void RTPClient::Stop() {
    _done = true;
    if (_communicationThread.joinable())
    {
        _communicationThread.join();
    }
}

void RTPClient::UnInit() {
    Stop();
    std::string reason("Client uninitialized.");
    _session.BYEDestroy(RTPTime(10,0), reason.c_str(), reason.size());
    _initialized = false;
}

void RTPClient::Connect() {
    uint8_t subtype = 0;
    const uint8_t name[4] = {'A','B','C','D'};
    std::string password = "HASLO";
    std::cout << "\nEnter password: ";
    std::cin >> password;
    std::string passHash = sha256(password);
    std::cout << "Password hash equals: " << passHash << std::endl;

    for (int i = 0; i < 15; i++)
    {
        int status = _session.SendRTCPAPPPacket(subtype, name, (void*)passHash.c_str(), passHash.length());
        CheckRTPError(status);
        std::cout << "Hash of the password was sent.\n";

        // waiting 2 seconds for response
        RTPTime::Wait(RTPTime(2,0));

        if (_session.serverResponded)
        {
            _connected = true;
            std::cout << "Password accepted.\n";
            break;
        }
    }

    if (!_connected)
        throw std::runtime_error("Could not connect to the server.\n");

}


