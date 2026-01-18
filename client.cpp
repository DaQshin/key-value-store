#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>

#define SERVER_PORT 5000
#define SERVER_ADDR "127.0.0.1"

int readResponses()


int main(){

    int client_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(client_fd < 0){
        perror("socket");
        return 1;
    }

    sockaddr_in client_addr;
    std::memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT);

    if(inet_pton(AF_INET, SERVER_ADDR, &client_addr.sin_addr) < 0){
        perror("inet_pton");
        return 1;
    }

    if(connect(client_fd, (sockaddr*)& client_addr, sizeof(client_addr)) < 0){
        perror("connect");
        return 1;
    }

    char buff[1024];
    if(recv(client_fd, buff, sizeof(buff) - 1, 0) < 0){
        perror("revc");
        return 1;
    }

    buff[sizeof(buff) - 1] = '\0';

    std::cout << "Server: " << buff << std::endl;

    close(client_fd);
}