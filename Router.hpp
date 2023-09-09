#include "Defines.hpp"

class Router 
{
public:

    Router() {}
    void communicate(int rPort);
    
    int selfSocket;
    ServerType serverType;
    int routerPort;
    ReceivedMessages receivedMessages;
    int serverId;
    int routerPortOut;
    int selfSocketOut;
    ReceivedPackets receivedPackets;
    ServerInfos serverInfos;

    void establish_socket_connection(int& socket_file_descriptor, int channel_port);
    char* getCharArrayOfPacket(Packet packet);
    Packet getPacketFromCharArray(char* charArray);
    void showError(string error);
    void addToReceivedMessages(Packet packet);
    void handle_new_connection(int &max_fd, fd_set &file_desc_set);
    void close_client_connection(int fd, fd_set& file_desc_set, int max_fd);
    void handle_client_command(int fd, int &max_fd, fd_set& file_desc_set);
    vector<char> getDigitsOfNumber(int number);
    int getNumberByDigits(char* digits);
    ServerInfo findFdOfServer(int sourceServer);
    void setServerId(int sId, int fd);

private:
};






// #ifndef ROUTER_HPP
// #define ROUTER_HPP

// #include <vector>
// #include <string>
// #include <cstring>
// #include <iostream>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <errno.h>
// #include<fstream>
// #include <sstream>
// #include <string>
// #include <bits/stdc++.h>
// #include <cstdint>
// #include <thread>

// using namespace std;

// const int SENDER = 1;
// const int RECEIVER = 2;
// const int PACKET_DATA_SIZE = 1500;
// enum ServerType { Sender = SENDER, Receiver = RECEIVER };
// const int MAX_WINDOW_SIZE = 10;
// const int ACK_TIMEOUT = 5;
// const int NUMBERS_SIZE = 7;
// const int PACKET_SIZE = PACKET_DATA_SIZE + NUMBERS_SIZE * 2 + 2;

// struct Packet
// {
//     char sourceServer;
//     int sequenceNumber;
//     int messageSize;
//     char ackFlag;
//     string data;
// };

// typedef vector<Packet> Message;
// typedef vector<Packet> ReceivedPackets;
// typedef vector<vector<int> > ServersConnectionData;

// class Router
// {
// public:

//     Router(int inputPort, int outputPort) : routerPort(inputPort), routerOutPort(outputPort) {}
//     void run();

    
//     ServersConnectionData serversData;
//     int selfSocket;
//     int selfSocketOut;
//     ServerType serverType;
//     int routerPort;
//     int routerOutPort;
//     ReceivedPackets receivedPackets;
//     int serverId;

//     void establish_socket_connection(int& socket_file_descriptor, int channel_port);
//     // void sendPackets();
//     // void receiveMessage();
//     Message readFile(string filename);
//     void splitLineToPackets(string line, Message &message);
//     char* getCharArrayOfPacket(Packet packet);
//     Packet getPacketFromCharArray(char* charArray);
//     void writeToFile(Message message, string filename);
//     void showError(string error);
//     void connect_socket(int& file_descriptor, int port);
//     void addToReceivedMessages(Packet packet);
//     void writeMessageToFile(int messageIndex);
//     void handle_new_connection(int &max_fd, fd_set &file_desc_set);
//     void close_client_connection(int fd, fd_set& file_desc_set, int max_fd);
//     void handle_client_command(int fd, int &max_fd, fd_set& file_desc_set);
//     vector<char> getDigitsOfNumber(int number);
//     int getNumberByDigits(char* digits);
//     void showMessageLogs(Message message);

// private:
// };

// #endif
