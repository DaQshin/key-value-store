#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <string>
#include <iostream>

#define SERVER_PORT 5000
#define SERVER_ADDR "127.0.0.1"

int readResponses(int fd, char* buff, size_t buff_size, int flag = 0){
    ssize_t n = recv(fd, buff, buff_size - 1, flag);
    if(n < 0){
        perror("recv");
        return -1;
    }

    buff[n] = '\0';

    return n;
}


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

    int ret = inet_pton(AF_INET, SERVER_ADDR, &client_addr.sin_addr);

    if(ret <= 0){
        if(ret < 0){
            perror("inet_pton");
            return 1;
        }

        std::cerr << "Invalid Address\n";
    }

    if(connect(client_fd, (sockaddr*)& client_addr, sizeof(client_addr)) < 0){
        perror("connect");
        return 1;
    }

    char buff[1024] = {0};
    
    if(readResponses(client_fd, buff, sizeof(buff), 0) > 0){
        std::cout << "Server: " << buff << std::endl;
    }

    close(client_fd);
}