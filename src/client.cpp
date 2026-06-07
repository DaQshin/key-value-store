#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include "logs/log.h"

#define PORT 5000

const size_t k_max_msg = 4096;

static void msg(const char* msg){
    fprintf(stderr, "%s\n", msg);
}

static void die(const char* msg){
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

enum {
    TAG_NIL = 0,
    TAG_ERR = 1,
    TAG_INT = 2,
    TAG_STR = 3,
    TAG_DBL = 4,
    TAG_ARR = 5,
};

static int32_t print_response(const uint8_t* data, size_t size){
    if(size < 1){
        msg("bad response");
        return -1;
    }

    switch(data[0]){
        case TAG_NIL:
            printf("(nil)\n");
            return 1;

        case TAG_ERR:
            {
                if(size < 1 + 8){
                    msg("bad response");
                    return -1;
                }
            }
            {
                int32_t code = 0;
                uint32_t len = 0;
                memcpy(&code, &data[1], 4);
                memcpy(&len, &data[1 + 4], 4);
                if(size < 1 + 8 + len){
                    msg("bad response");
                    return -1;
                }
                printf("(err) %d %.*s\n", code, len, &data[1 + 8]);
                return 1 + 8 + len;
            }
        
        case TAG_STR:
            {
                if(size < 1 + 4){
                    msg("bad response");
                    return -1;
                }
            }
            {
                uint32_t len = 0;
                memcpy(&len, &data[1], 4);
                if(size < 1 + 4 + len){
                    msg("bad response");
                    return -1;
                }

                printf("(str) %d %.*s \n", len, len, &data[1 + 4]);
                return 1 + 4 + len;
            }

        case TAG_INT:
            {
                if(size < 1 + 8){
                    msg("bad response");
                    return -1;
                }
            }
            {
                int64_t val = 0;
                memcpy(&val, &data[1], 8);
                printf("(int) %ld\n", val);
                return 1 + 8;
            }

        case TAG_DBL:
            {
                if(size < 1 + 8){
                    msg("bad response");
                    return -1;
                }
            }
            {
                double val = 0;
                memcpy(&val, &data[1], 8);
                printf("(double) %g\n", val);
                return 1 + 8;
            }
        
        default:
            msg("bad response");
            return -1;
    };
}

static int32_t read_full(int fd, uint8_t* read_buffer, size_t n){
    while(n > 0){
        ssize_t rv = read(fd, read_buffer, n);
        // LOG_DEBUG("Bytes read: %zd", rv);
        if(rv <= 0){
            return -1;
        }

        assert((size_t)rv <= n);
        n -= (size_t)rv;
        read_buffer += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const uint8_t* write_buffer, size_t n){
    while(n > 0){
        ssize_t rv = write(fd, write_buffer, n);
        // LOG_DEBUG("bytes written: %zd", rv);
        if(rv <= 0){
            return -1;
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        write_buffer += rv;
    }

    return 0;
}

static int send_req(int fd, std::vector<std::string>& cmd){
    uint32_t len = 4;
    for(const std::string& s: cmd){
        len += 4 + s.size();
    }

    if(len > k_max_msg) return -1;

    uint8_t wbuf[4 + k_max_msg];
    memcpy(&wbuf[0], &len, 4);
    uint32_t nstr = cmd.size();
    memcpy(&wbuf[4], &nstr, 4);
    uint32_t cur = 8;
    for(const std::string& s: cmd){
        uint32_t n = (uint32_t)s.size();
        memcpy(&wbuf[cur], &n, 4);
        memcpy(&wbuf[cur + 4], s.data(), n);
        cur += 4 + n;
    }

    return write_all(fd, wbuf, 4 + len);
}

static int32_t read_res(int fd){
    uint8_t rbuf[4 + k_max_msg];
    errno = 0;
    int32_t err = read_full(fd, rbuf, 4);
    if(err){
        if(errno == 0){
            msg("EOF");
        }
        else msg("read() error");
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, &rbuf[0], 4);
    if(len > k_max_msg){
        msg("msg too long");
        return -1;
    }

    err = read_full(fd, &rbuf[4], len);
    if(err){
        msg("read() error");
        return err;
    }

    int32_t rv = print_response((const uint8_t*)&rbuf[4], len);
    if(rv > 0 && (uint32_t)rv != len){
        msg("bad response");
        rv = -1;
    }

    return rv;
}

int32_t parse_cmd(std::vector<std::string>& cmd, const std::string& line){
    std::istringstream stream(line);
    std::string token;
    
    while(stream >> token){
        cmd.push_back(token);
    }

    return 0;
}

int main(int argc, char** argv){
    const char* host = "127.0.0.1";
    for(int i = 1; i < argc; i++){
        if(std::string(argv[i]) == "--host"){
            if(i + 1 < argc) host = argv[i + 1];
            else die("hostname not provided");
        }
    }

    int client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if(client_fd < 0){
        die("socket()");
    }

    struct addrinfo addr{};
    addr.ai_family = AF_INET;
    addr.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = nullptr;

    int rv = getaddrinfo(host, "5000", &addr, &res);
    if(rv < 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        die("getaddrinfo()");
    }

    if(connect(client_fd, res->ai_addr, res->ai_addrlen) < 0){
        freeaddrinfo(res);
        close(client_fd);
        die("connect()");
    }

    struct pollfd poll_args[2];
    poll_args[0].fd = STDIN_FILENO;
    poll_args[0].events = POLLIN;
    poll_args[1].fd = client_fd;
    poll_args[1].events = POLLIN | POLLRDHUP;

    bool waiting_for_response = false;

    while(true){

        if(!waiting_for_response) std::cout << "cacheline> " << std::flush;

        int n = poll(poll_args, 2, -1);

        if(n < 0){
            if(errno == EINTR) continue;
            die("poll()");
        }

        int stdin_ready = poll_args[0].revents;
        int conn_ready = poll_args[1].revents;

        if(stdin_ready & POLLIN){
            std::string line;

            if(!std::getline(std::cin, line)) break;

            std::vector<std::string> commands;

            if(line == "exit()") goto CLEAN;

            if(parse_cmd(commands, line) < 0){
                std::cout << "Invalid Command" << std::endl; 
                continue;
            }
            
            if(send_req(client_fd, commands) < 0) {
                LOG_ERROR("Connection Failed");
                goto CLEAN;
            }
            waiting_for_response = true;

        }

        if(conn_ready & POLLIN && waiting_for_response){
            if(read_res(client_fd) < 0){
                LOG_ERROR("Connection Terminated.");
                continue;
            }
            waiting_for_response = false;
        }

        if(conn_ready & (POLLRDHUP | POLLHUP | POLLERR)){
            std::cout << "Connection Terminated." << std::endl;
            goto CLEAN;
        }
    }
    CLEAN:
        close(client_fd);
        return 0;

}