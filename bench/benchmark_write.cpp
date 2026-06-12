#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <iostream>
#include "client.h"

int main(){

    KVClient client("127.0.0.1", "5000");

    if(!client.open()){
        std::cerr << "open() failed\n";
        return 1;
    }

    int N = 10000;

    auto start = std::chrono::steady_clock::now();

    // for(int i = 0; i < 10000; i++){
    //     if(!client.send({
    //         "SET",
    //         "key" + std::to_string(i),
    //         "value"
    //     })){
    //         std::cerr << "send() failed\n";
    //         return 1;
    //     }

    //     // if(!client.recieve()){
    //     //     std::cerr << "recieve() failed\n";
    //     //     return 1;
    //     // }
    // }


    for(int i = 0; i < N; i++) {
        if(!client.send({
            "SET",
            "key" + std::to_string(i),
            "value"
        })){
            std::cerr << "send() failed\n";
            return 1;
        }
    }

    for(int i = 0; i < N; i++) {
        if(!client.receive()){
            std::cerr << "recieve() failed\n";
            return 1;
        }
    }

    auto end = std::chrono::steady_clock::now();

    auto elapsed_time = 
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double seconds = elapsed_time.count() / 1000.0;
    double rps = 10000.0 / seconds;


    std::cout << "Elapsed:" << seconds << " sec\n";
    std::cout << "Requests/second: " << rps << '\n';

}