#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <fcntl.h>
#include <cerrno>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>

#define PORT 5000

int sockopt = 1;
fd_set fr, fw, fe;
std::vector<int> fds;

int setNBIO(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags == -1){
        perror("fcntl F_GETFL");
        return -1;
    }

    if(fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1){
        perror("fcntl F_SETFL");
        return -1;
    }

    return 0;

}

void sendResponse(std::string msg, int fd, int flag = 0){
    if(send(fd, msg.c_str(), msg.size(), flag) < 0){
        perror("send");
    }
}

int acceptRequests(int server_fd){
    if(FD_ISSET(server_fd, &fr)){
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (sockaddr*)& client_addr, &client_len);

        if(client_fd < 0){
            if(errno != EWOULDBLOCK && errno != EAGAIN){
                perror("accept");
            }
            return -1;
        }  


        setNBIO(client_fd);
        fds.push_back(client_fd);

        std::string msg = "hello, client!";

        sendResponse(msg, client_fd, MSG_NOSIGNAL);

        std::cout << "client connected\n {fd = " << client_fd << "}" << std::endl;

    }

    return 0;
}


int main(){

    int server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(server_fd < 0){
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    setNBIO(server_fd);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

    if(bind(server_fd, (sockaddr*)& server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        return 1;
    }

    if(listen(server_fd, 10) < 0){
        perror("listen");
        return 1;
    }

    std::cout << "Server listening at port " << PORT << std::endl;

    while(1){
        FD_ZERO(&fr);

        FD_SET(server_fd, &fr);

        int max_fd = server_fd;
        for(int fd: fds){
            FD_SET(fd, &fr);
            max_fd = std::max(max_fd, fd);
        }

        if(select(max_fd + 1, &fr, nullptr, nullptr, nullptr) < 0){
            if(errno == EINTR) continue;
            perror("select");
            break;
        }

        acceptRequests(server_fd);
    }
}