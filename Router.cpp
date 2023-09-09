#include "Router.hpp"

void receiveMessage(Router* router);

void sendMessage(Router* router) 
{
    while(true)
    {
        for (int i = 0; i < router->receivedPackets.size(); i++)
        {
            ServerInfo serverInfo = router->findFdOfServer((int)router->receivedPackets[i].destinationServer);
            char* sendingBuffer = router->getCharArrayOfPacket(router->receivedPackets[i]);
            send(serverInfo.serverFd, sendingBuffer, PACKET_SIZE, 0);
            router->receivedPackets.erase(router->receivedPackets.begin() + i);
            cout << "ROUTER" << " --> packet " << router->receivedPackets[i].sequenceNumber << " sent to server " << (int)router->receivedPackets[i].destinationServer << endl;
        }
    }
}

void Router::close_client_connection(int fd, fd_set& file_desc_set, int max_fd){
    close(fd);
    FD_CLR(fd, &file_desc_set);
    if (fd == max_fd)
        while (FD_ISSET(max_fd, &file_desc_set) == 0)
            max_fd -= 1;
}

void Router::handle_new_connection(int &max_fd, fd_set &file_desc_set){

    int new_command_socket = accept(this->selfSocket, NULL, NULL);
    if (new_command_socket < 0){
        throw runtime_error("ERROR: error in new command socket");
    }

    FD_SET(new_command_socket, &file_desc_set);
    if (new_command_socket > max_fd)
        max_fd = new_command_socket;
    ServerInfo serverInfo;
    serverInfo.sourceSet = false;
    serverInfo.serverFd = new_command_socket;
    this->serverInfos.push_back(serverInfo);
}

void Router::setServerId(int sId, int fd)
{
    for (int i = 0; i < this->serverInfos.size(); i++)
        if (this->serverInfos[i].serverFd == fd)
        {
            this->serverInfos[i].serverId = sId;
            return;
        }
}

ServerInfo Router::findFdOfServer(int sourceServer)
{
    for (auto serverInfo : this->serverInfos)
        if (serverInfo.serverId == sourceServer)
            return serverInfo;
    ServerInfo nullServerInfo;
    nullServerInfo.sourceSet = false;
    nullServerInfo.serverFd = -1;
    nullServerInfo.serverId = 0;
    return nullServerInfo;
}

void Router::handle_client_command(int fd, int &max_fd, fd_set& file_desc_set){
    char received_buffer[PACKET_SIZE] = {0};
    memset(received_buffer, 0, PACKET_SIZE);
    int result = recv(fd, received_buffer, PACKET_SIZE, 0);
    // if (result <= 0)
    // {
    //     cout << "ERROR: rec error" << endl;
    //     close_client_connection(fd, file_desc_set, max_fd);
    // }
    if (result > 0) 
    {
        Packet packet = getPacketFromCharArray(received_buffer);
        setServerId((int)packet.sourceServer, fd);
        if (packet.messageSize == 0)
        {
            cout << "ROUTER" << " --> Handshaking done with receiver server " << (int)packet.sourceServer << endl;
            return;
        }
        addToReceivedMessages(packet);
    }
}

void receiveMessage(Router* router)
{
    fd_set read_fd_set, file_desc_set;
    FD_ZERO(&file_desc_set);
    FD_SET(router->selfSocket, &file_desc_set);
    int max_fd = router->selfSocket;
    cout << "ROUTER"<< " --> Router is running on port " << router->routerPort << endl;
    while (true) {
        memcpy(&read_fd_set, &file_desc_set, sizeof(file_desc_set));
        int ready_sockets_num = select(max_fd + 1, &read_fd_set, NULL, NULL, NULL);
        for (int fd = 0; fd <= max_fd  &&  ready_sockets_num > 0; ++fd) {
            if (FD_ISSET(fd, &read_fd_set)) {
                if (fd == router->selfSocket) {
                    router->handle_new_connection(max_fd, file_desc_set);
                }
                else {
                    router->handle_client_command(fd, max_fd, file_desc_set);
                }
            }
        }
    }
}

void Router::addToReceivedMessages(Packet packet)
{
    if (rand() % 100 + 1 < PACKET_DROP_RATE)
    {
        cout << "ROUTER" << " --> packet " << packet.sequenceNumber << " from server " << (int)packet.sourceServer << " was dropped" << endl;
        return;
    }
    cout << "ROUTER" << " --> packet " << packet.sequenceNumber << " received from server " << (int)packet.sourceServer << endl;
    receivedPackets.push_back(packet);
}

void Router::establish_socket_connection(int& socket_file_descriptor, int channel_port) {
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_file_descriptor < 0 )
        throw runtime_error("cannot create socket");

    int opt = 1;
    if (setsockopt(socket_file_descriptor, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        throw runtime_error("ERROR: error in setsockopt");

    struct sockaddr_in socket_file_descriptor_address;
    socket_file_descriptor_address.sin_family = AF_INET;
    socket_file_descriptor_address.sin_addr.s_addr = INADDR_ANY;
    socket_file_descriptor_address.sin_port = htons(channel_port);

    if (bind(socket_file_descriptor, (struct sockaddr *)&socket_file_descriptor_address, sizeof(socket_file_descriptor_address)) < 0) 
        throw runtime_error("ERROR: error in binding the socket");

    if (listen(socket_file_descriptor, 10) < 0)
        throw runtime_error("ERROR: error in listening on socket");
}

void Router::communicate(int rPort)
{
    this->routerPort = rPort;
    this->establish_socket_connection(this->selfSocket, this->routerPort);

    std::thread s(sendMessage, this);
    std::thread r(receiveMessage, this);

    s.join();
    r.join();
}

vector<char> Router::getDigitsOfNumber(int number)
{
    vector<char> digits(NUMBERS_SIZE, 0);
    for (int i = 0; i < NUMBERS_SIZE; i++)
    {
        digits[i] = number % 10;
        number /= 10;
    }
    return digits;
}

int Router::getNumberByDigits(char* digits)
{
    int number = 0;
    for (int i = 0; i < NUMBERS_SIZE; i++)
        number += (int)pow(10, i) * (int)(digits[i]);
    return number;
}

char* Router::getCharArrayOfPacket(Packet packet)
{
    char* charArray = new char[PACKET_SIZE];
    for (int i = 0; i < PACKET_SIZE; i++)
        charArray[i] = '\0';
    charArray[0] = packet.sourceServer;
    charArray[1] = packet.ackFlag;
    vector<char> digitsSeq = getDigitsOfNumber(packet.sequenceNumber);
    for (int i = 2; i < NUMBERS_SIZE + 2; i++)
        charArray[i] = digitsSeq[i - 2];
    vector<char> digitsMSize = getDigitsOfNumber(packet.messageSize);
    for (int i = NUMBERS_SIZE + 2; i < 2 * NUMBERS_SIZE + 2; i++)
        charArray[i] = digitsMSize[i - NUMBERS_SIZE - 2];
    for (int i = 2 * NUMBERS_SIZE + 2; i < PACKET_SIZE; i++)
        charArray[i] = packet.data[i - (2 * NUMBERS_SIZE + 2)];
    charArray[PACKET_SIZE - 1] = packet.destinationServer;
    return charArray;
}

Packet Router::getPacketFromCharArray(char* charArray)
{
    Packet packet;
    packet.sourceServer = charArray[0];
    packet.ackFlag = charArray[1];
    packet.sequenceNumber = getNumberByDigits(charArray + 2);
    packet.messageSize = getNumberByDigits(charArray + NUMBERS_SIZE + 2);
    string data = "";
    for (int i = 2 * NUMBERS_SIZE + 2; i < PACKET_SIZE; i++)
        data = data + charArray[i];
    packet.data = data;
    packet.destinationServer = charArray[PACKET_SIZE - 1];
    return packet;
}

void Router::showError(string error)
{
    cerr << error << endl;
}

int32_t main(int argsCount,char *argsList[])
{
    Router router;
    router.communicate(COMMUNICATION_PORT);
    return 0;
}
