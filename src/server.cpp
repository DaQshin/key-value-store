#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

#define PORT 5000

static void msg(const char* msg){
    fprintf(stderr, "%s\n", msg);
}

static void error_handler(const char* msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

const size_t k_max_msg = 4096;

static int32_t read_full(int fd, char* buffer, size_t n){
    while(n){
        ssize_t rv = read(fd, buffer, n);
        if(rv <= 0) return -1;

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buffer += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const char* buffer, size_t n){
    while(n){
        ssize_t rv = write(fd, buffer, n);
        if(rv <= 0) return -1;

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buffer += rv;
    }

    return 0;
}

static int32_t one_request(int connfd){
    char read_buffer[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(connfd, read_buffer, 4);
    if(err){
        msg(errno == 0 ? "EOF" : "read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, read_buffer, 4);
    if(len > k_max_msg){
        msg("message length too long");
        return -1;
    }

    err = read_full(connfd, &read_buffer[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    fprintf(stderr, "Client: %.*s\n", len, &read_buffer[4]);

    const char reply[] = "what is happening";
    char write_buffer[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);
    memcpy(write_buffer, &len, 4);
    memcpy(&write_buffer[4], reply, len);

    int32_t rv = write_all(connfd, write_buffer, 4 + len);

    return rv;
}

// static void rwtoclient(int connfd, const char* write_buffer){
//     char read_buffer[64];
//     ssize_t n = read(connfd, read_buffer, sizeof(read_buffer) - 1);
//     if(n < 0){
//         msg("read() error");
//         return;
//     }

//     fprintf(stderr, "Client: %s\n", read_buffer);

//     write(connfd, write_buffer, sizeof(write_buffer) - 1);
// }

int main(){
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0){
        error_handler("socket()");
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int val = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0){
        error_handler("setsockopt()");
    }

    if(bind(fd, (sockaddr*)& server_addr, sizeof(server_addr)) < 0){
        error_handler("bind()");
    }

    if(listen(fd, 10) < 0){
        error_handler("listen()");
    }

    std::cout << "Server running at port:" << PORT << std::endl;

    while(1){
        sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        int connfd = accept(fd, (sockaddr*)& client_addr, &addrlen);
        if(connfd < 0 ){
            continue;
        }

        while(1){
            int32_t err = one_request(connfd);
            if(err) break;
        }
        close(connfd);
    }

    return 0;

}