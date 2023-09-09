#include "Defines.hpp"

class Server 
{
public:

    Server(ServerType type, int id) : serverType(type) , serverId(id) {}
    void communicate(int rPort);

    int selfSocket;
    ServerType serverType;
    int routerPort;
    ReceivedMessages receivedMessages;
    int serverId;
    SlidingWindow window;

    void establish_socket_connection(int& socket_file_descriptor, int channel_port);
    Message readFile(string filename);
    void splitLineToPackets(string line, Message &message);
    char* getCharArrayOfPacket(Packet packet);
    Packet getPacketFromCharArray(char* charArray);
    void writeToFile(Message message, string filename);
    void showError(string error);
    void connect_socket(int& file_descriptor, int port);
    void addToReceivedMessages(Packet packet);
    void writeMessageToFile(int messageIndex);
    void handle_new_connection(int &max_fd, fd_set &file_desc_set);
    void close_client_connection(int fd, fd_set& file_desc_set, int max_fd);
    void handle_client_command(int fd, int &max_fd, fd_set& file_desc_set);
    vector<char> getDigitsOfNumber(int number);
    int getNumberByDigits(char* digits);
    void showMessageLogs(Message message);
    void sendHandShaking();
    void sendAck(Packet packet);
    bool checkIfAllPacketsReceived(int messageIndex);
    void checkIfAckIsInTime(Packet packet);
    void resendWindow();
    bool checkIfExist(Packet packet);

private:
};
