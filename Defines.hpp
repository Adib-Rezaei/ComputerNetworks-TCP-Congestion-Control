#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include<fstream>
#include <sstream>
#include <string>
#include <bits/stdc++.h>
#include <stdexcept>

using namespace std;

const int COMMUNICATION_PORT = 8080;
const int SENDER = 1;
const int RECEIVER = 2;
const int PACKET_DATA_SIZE = 1500;
enum ServerType { Sender = SENDER, Receiver = RECEIVER };
const int ACK_TIMEOUT = 5;
const int NUMBERS_SIZE = 7;
const int PACKET_SIZE = PACKET_DATA_SIZE + NUMBERS_SIZE * 2 + 3;
const int ACK_WAITING_TIME = 100000;
const int RECEIVER_SERVER_ID = 1;
const int PACKET_DROP_RATE = 50;
const int MAX_WINDOW_SIZE = 2;

struct ServerInfo
{
    bool sourceSet;
    int serverId;
    int serverFd;
};

typedef struct ServerInfo ServerInfo;
typedef vector<ServerInfo> ServerInfos;

struct Packet
{
    char destinationServer;
    char sourceServer;
    int sequenceNumber;
    int messageSize;
    char ackFlag;
    string data;
};

typedef struct Packet Packet;
typedef vector<Packet> Message;
typedef vector<Message> ReceivedMessages;
typedef vector<Packet> ReceivedPackets;

struct PacketSentData
{
    Packet packet;
    clock_t sentTime;
};

typedef struct PacketSentData PacketSentData;
typedef vector<PacketSentData> SlidingWindow;