#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <array>
#include <memory>

#define PORT 6969
#define REC_PORT 6868
#define BUFFER_SIZE 256

void receive(int sock, sockaddr_in addr){
    std::cout << "\nstart listening\n";
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, sizeof(buffer));
    socklen_t addrLen = sizeof(addr);
    while(1){
        ssize_t bytesReceived = recvfrom(sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addrLen);
        if(bytesReceived > 0){
            std::cout << buffer << std::endl;
            std::cout << "received some data\n";
        }
        memset(buffer, 0, sizeof(buffer));
    }
}

std::string exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, int (*)(FILE *)> pipe(popen(cmd, "r"), pclose);
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

std::string getBroadCastAddress(){
    return exec("INTERFACE=$(ip route show default | awk \'{print $5}\') && ip -4 addr show $INTERFACE | awk \'$1 == \"inet\" {print $4}'");
}

void setColor(int r, int g, int b) {
    std::cout << "\033[38;2;" << r << ";" << g << ";" << b << "m";
}

void resetColor() {
    std::cout << "\033[0m";
}

int main() {
    int sock;
    int opt = 1;
    struct sockaddr_in broadcastAddr;

    // sending
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }
    //enable broadcast
    if (setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        close(sock);
        return -1;
    }

    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(PORT);
    broadcastAddr.sin_addr.s_addr = inet_addr(getBroadCastAddress().c_str()); 

    //receiving
    int rec_sock;
    struct sockaddr_in broadcastAddr_rec;
    if ((rec_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cout << "receving socket error";
        close(rec_sock);
        return -1;
    }

    memset(&broadcastAddr_rec, 0, sizeof(broadcastAddr_rec));
    broadcastAddr_rec.sin_family = AF_INET;
    broadcastAddr_rec.sin_addr.s_addr = INADDR_ANY;
    broadcastAddr_rec.sin_port = htons(REC_PORT);

    if (bind(rec_sock, (struct sockaddr*)&broadcastAddr_rec, sizeof(broadcastAddr_rec)) < 0) {
        std::cout << "receiving binding err";
        close(rec_sock);
        return -1;
    }

    std::cout << "W3LC0M3 T0\n";
    setColor(255, 0, 255);
    std::cout << 
"     _____     __          ____                  __  \n" <<
"   / ____/____/ /_  ____  / __ )________  ____ _/ /__\n" <<
"  / __/ / ___/ __ \\/ __ \\/ __  / ___/ _ \\/ __ `/ //_/\n" <<
" / /___/ /__/ / / / /_/ / /_/ / /  /  __/ /_/ / ,<   \n" <<
"/_____/\\___/_/ /_/\\____/_____/_/   \\___/\\__,_/_/|_| \033[0m\n\n"; 
resetColor();
    std::string message;
    std::cout << "Enter the message in format(hostname;cmd/func;command) to exit write exit: ";
    socklen_t addrLen = sizeof(broadcastAddr_rec);

    std::thread rec(receive, rec_sock, broadcastAddr_rec);

    while(true){
        //sending
        std::getline(std::cin, message);
        if(message == "exit"){
            break;
        }
        sendto(sock, message.c_str(), message.size(), 0, (struct sockaddr*)&broadcastAddr, sizeof(broadcastAddr));
        message = "";
    }
    rec.join();
    close(sock);
    return 0;
}
