#include "Server.hpp"

void sendMessage(Server* server) 
{
    if (server->serverType == Receiver)
        return;
    Message message = server->readFile("toSend.txt");
    while (true)
    {
        while (message.size() > 0 && server->window.size() < MAX_WINDOW_SIZE)
        {
            message[0].destinationServer = RECEIVER_SERVER_ID;
            char* sendingBuffer = server->getCharArrayOfPacket(message[0]);

            PacketSentData sentData;
            sentData.packet = message[0];
            sentData.sentTime = clock();

            send(server->selfSocket, sendingBuffer, PACKET_SIZE, 0);
            cout << "SERVER "<< server->serverId << " --> packet " << message[0].sequenceNumber << " sent to server " << (int)message[0].destinationServer << endl;
            
            server->window.push_back(sentData);
            message.erase(message.begin());
        }
        for (auto pacSentData : server->window)
            if (clock() - pacSentData.sentTime > ACK_WAITING_TIME)
                server->resendWindow();
    }
}

void Server::close_client_connection(int fd, fd_set& file_desc_set, int max_fd){
    close(fd);
    FD_CLR(fd, &file_desc_set);
    if (fd == max_fd)
        while (FD_ISSET(max_fd, &file_desc_set) == 0)
            max_fd -= 1;
}

void Server::handle_new_connection(int &max_fd, fd_set &file_desc_set){
    int new_command_socket = accept(this->selfSocket, NULL, NULL);
    if (new_command_socket < 0){
        throw runtime_error("ERROR: error in new command socket");
    }

    FD_SET(new_command_socket, &file_desc_set);
    if (new_command_socket > max_fd)
        max_fd = new_command_socket;
}

void Server::handle_client_command(int fd, int &max_fd, fd_set& file_desc_set){
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
        addToReceivedMessages(packet);
    }
}

void receiveMessage(Server* server)
{
    server->sendHandShaking();
    fd_set read_fd_set, file_desc_set;
    FD_ZERO(&file_desc_set);
    FD_SET(server->selfSocket, &file_desc_set);
    int max_fd = server->selfSocket;
    cout << "SERVER "<< server->serverId << " --> Server is running on port " << server->routerPort << endl;
    while (true) {
        // memcpy(&read_fd_set, &file_desc_set, sizeof(file_desc_set));
        // int ready_sockets_num = select(max_fd + 1, &read_fd_set, NULL, NULL, NULL);
        // for (int fd = 0; fd <= max_fd  &&  ready_sockets_num > 0; ++fd) {
        //     if (FD_ISSET(fd, &read_fd_set)) {
        //         if (fd == this->selfSocket) {
        //             handle_new_connection(max_fd, file_desc_set);
        //         }
        //         else {
        server->handle_client_command(server->selfSocket, max_fd, file_desc_set);
        //         }
        //     }
        // }
    }
}

void Server::sendAck(Packet packet)
{
    cout << "SERVER "<< serverId << " --> packet " << packet.sequenceNumber << " acknowledge sent to server " << (int)packet.sourceServer << endl;
    Packet ackPacket;
    ackPacket.sourceServer = this->serverId;
    ackPacket.messageSize = 1;
    ackPacket.sequenceNumber = packet.sequenceNumber;
    ackPacket.ackFlag = 1;
    char temp[PACKET_DATA_SIZE] = {0};
    ackPacket.data = temp;
    ackPacket.destinationServer = packet.sourceServer;
    char* sendingBuffer = getCharArrayOfPacket(ackPacket);
    send(this->selfSocket, sendingBuffer, PACKET_SIZE, 0);
}

void Server::resendWindow()
{
    for (int j = 0; j < this->window.size(); j++)
    {
        this->window[j].packet.destinationServer = RECEIVER_SERVER_ID;
        char* sendingBuffer = this->getCharArrayOfPacket(this->window[j].packet);
        this->window[j].sentTime = clock();
        send(this->selfSocket, sendingBuffer, PACKET_SIZE, 0);
        cout << "SERVER "<< this->serverId << " --> packet " << this->window[j].packet.sequenceNumber << " sent to server " << (int)this->window[j].packet.destinationServer << " again " << endl;
    }
}

void Server::checkIfAckIsInTime(Packet packet)
{
    for (int i = 0; i < this->window.size(); i++)
    {
        if (this->window[i].packet.sequenceNumber == packet.sequenceNumber)
        {
            if (clock() - this->window[i].sentTime <= ACK_WAITING_TIME)
                this->window.erase(this->window.begin() + i);
            else 
                this->resendWindow();
        }
    }
}

bool Server::checkIfExist(Packet packet)
{
    for (auto mes : this->receivedMessages)
        for (auto pac : mes)
            if (pac.sourceServer == packet.sourceServer && pac.sequenceNumber == packet.sequenceNumber)
                return true;
        
}

void Server::addToReceivedMessages(Packet packet)
{
    if ((int)packet.ackFlag == 1)
    {
        cout << "SERVER "<< serverId << " --> packet " << packet.sequenceNumber << " acknowledge received from server " << (int)packet.sourceServer << endl;
        checkIfAckIsInTime(packet);
        return;
    }
    cout << "SERVER "<< serverId << " --> packet " << packet.sequenceNumber << " received from server " << (int)packet.sourceServer << endl;
    sendAck(packet);
    if (checkIfExist(packet))
        return;
    for (int i = 0; i < this->receivedMessages.size(); i++)
        if (this->receivedMessages[i][0].sourceServer == packet.sourceServer)
        {
            this->receivedMessages[i].push_back(packet);
            writeMessageToFile(i);
            return;
        }
    Message newMessage;
    newMessage.push_back(packet);
    this->receivedMessages.push_back(newMessage);
    writeMessageToFile(receivedMessages.size() - 1);
}

bool Server::checkIfAllPacketsReceived(int messageIndex)
{
    for (int i = 0; i < receivedMessages[messageIndex][0].messageSize; i++)
    {
        bool received = false;
        for (auto packet : receivedMessages[messageIndex])
            if (i == packet.sequenceNumber)
                received = true;
        if (!received)
            return false;
    }
    return true;
}


void Server::writeMessageToFile(int messageIndex)
{
    if (this->checkIfAllPacketsReceived(messageIndex))
    {
        writeToFile(receivedMessages[messageIndex], "received_form_server_" + to_string(receivedMessages[messageIndex][0].sourceServer) + "_.txt");
        receivedMessages.erase(receivedMessages.begin() + messageIndex);
    }
}

void Server::showMessageLogs(Message message)
{
    cout << endl;
    cout << "##############################################" << endl;
    cout << "packet.ackFlag: " << (int)message[0].ackFlag << endl;
    cout << "packet.messageSize: " << (int)message[0].messageSize << endl;
    cout << "packet.sequenceNumber: " << (int)message[0].sequenceNumber << endl;
    cout << "packet.sourceServer: " << (int)message[0].sourceServer << endl;
    cout << "##############################################" << endl;
    cout << endl;
}

void Server::connect_socket(int& file_descriptor, int port) {
    file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (file_descriptor < 0)
        throw runtime_error("ERROR: cannot create socket");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
        throw runtime_error("ERROR: error in inet_pton");

    if (connect(file_descriptor, (struct sockaddr*)&server_address, sizeof(server_address)) < 0)
        throw runtime_error("ERROR: error in connecting client");
}

void Server::establish_socket_connection(int& socket_file_descriptor, int channel_port) {
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

void Server::sendHandShaking()
{
    Packet shakeHandPacket;
    shakeHandPacket.sourceServer = this->serverId;
    shakeHandPacket.messageSize = 0;
    shakeHandPacket.sequenceNumber = 0;
    shakeHandPacket.ackFlag = 0;
    char temp[PACKET_DATA_SIZE] = {0};
    shakeHandPacket.data = temp;
    char* sendingBuffer = getCharArrayOfPacket(shakeHandPacket);
    send(this->selfSocket, sendingBuffer, PACKET_SIZE, 0);
}

void Server::communicate(int rPort)
{
    this->routerPort = rPort;
    connect_socket(this->selfSocket, this->routerPort);

    std::thread s(sendMessage, this);
    std::thread r(receiveMessage, this);

    s.join();
    r.join();
}

Message Server::readFile(string filename)
{
    ifstream file(filename);
    if (!file.is_open())
        showError("ERROR: opening file");
    string line;
    Message message;
    getline(file, line, '\0');
    splitLineToPackets(line, message);
    return message;
}

void Server::splitLineToPackets(string line, Message &message)
{
    int seqNumber = 0;
    for (int j = 0; j < line.length() / PACKET_DATA_SIZE + 1; j++)
    {
        Packet packet;
        packet.sourceServer = serverId;
        packet.sequenceNumber = seqNumber;
        packet.ackFlag = 0;
        packet.messageSize = line.length() / PACKET_DATA_SIZE + 1;
        packet.data = line.substr(j * PACKET_DATA_SIZE, PACKET_DATA_SIZE);
        message.push_back(packet);
        seqNumber++;
    }
}

vector<char> Server::getDigitsOfNumber(int number)
{
    vector<char> digits(NUMBERS_SIZE, 0);
    for (int i = 0; i < NUMBERS_SIZE; i++)
    {
        digits[i] = number % 10;
        number /= 10;
    }
    return digits;
}

int Server::getNumberByDigits(char* digits)
{
    int number = 0;
    for (int i = 0; i < NUMBERS_SIZE; i++)
        number += (int)pow(10, i) * (int)(digits[i]);
    return number;
}

char* Server::getCharArrayOfPacket(Packet packet)
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

Packet Server::getPacketFromCharArray(char* charArray)
{
    Packet packet;
    packet.sourceServer = charArray[0];
    packet.ackFlag = charArray[1];
    packet.sequenceNumber = getNumberByDigits(charArray + 2);
    packet.messageSize = getNumberByDigits(charArray + NUMBERS_SIZE + 2);
    string data = "";
    for (int i = 2 * NUMBERS_SIZE + 2; i < 2 * NUMBERS_SIZE + 2 + PACKET_DATA_SIZE; i++)
        data = data + charArray[i];
    packet.data = data;
    packet.destinationServer = charArray[PACKET_SIZE - 1];
    return packet;
}

void Server::writeToFile(Message message, string filename)
{
    ofstream file(filename);
    if (!file.is_open())
        showError("ERROR opening file");
    cout << endl << "--> Successfully wrote to file: " << filename << endl;
    showMessageLogs(message);
    for (int i = 0; i < message[0].messageSize - 1; i++)
    {
        for (auto packet : message)
            if (packet.sequenceNumber == i)
                file << packet.data;
    }
    for (auto character : message[message[0].messageSize - 1].data)
        if (character == '\0')
            return;
        else
            file << character;
}

void Server::showError(string error)
{
    cerr << error << endl;
}

int32_t main(int argsCount,char *argsList[])
{
    ServerType serverType = (atoi(argsList[1]) == SENDER ? Sender : Receiver);
    Server server(serverType, atoi(argsList[2]));
    server.communicate(COMMUNICATION_PORT);
    return 0;
}
