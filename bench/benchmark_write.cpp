#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <chrono>
#include <vector>
#include <iostream>
#include <algorithm>
#include "client.h"

double percentile(std::vector<double>& v, double p){
    std::sort(v.begin(), v.end());
    size_t idx = (size_t)(v.size() * p);
    return v[idx];
}

int main(){

    KVClient client("127.0.0.1", "5000");

    if(!client.open()){
        std::cerr << "open() failed\n";
        return 1;
    }

    int N = 10000;
    double cum_latency_send = 0.0;
    double cum_latency_receive = 0.0;
    std::vector<double> send_latencies;
    std::vector<double> recieve_latencies;
    std::vector<std::string> keys(N, "");

    send_latencies.reserve(N);
    recieve_latencies.reserve(N);

    for(int i = 0; i < N; i++){
        keys[i] += "key" + std::to_string(i);
    }

    // db warmup
    for (int i = 0; i < 1000; i++) {
        client.send({"SET", "warmup", "value"});
        client.receive();
    }

    auto start = std::chrono::steady_clock::now();

    for(int i = 0; i < 10000; i++){

        auto t1 = std::chrono::steady_clock::now();

        if(!client.send({
            "SET",
            keys[i],
            "value"
        })){
            std::cerr << "send() failed\n";
            return 1;
        }

        auto t2 = std::chrono::steady_clock::now();

        if(!client.receive()){
            std::cerr << "recieve() failed\n";
            return 1;
        }

        auto t3 = std::chrono::steady_clock::now();

        send_latencies[i] = std::chrono::duration<double, std::milli>(t2 - t1).count();
        recieve_latencies[i] = std::chrono::duration<double, std::milli>(t3 - t2).count();

        cum_latency_receive += recieve_latencies[i];
        cum_latency_send += send_latencies[i];
    }

    auto end = std::chrono::steady_clock::now();

    auto elapsed_time = 
    std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double seconds = elapsed_time.count() / 1000.0;
    double rps = 10000.0 / seconds;

    double ls_p50 = percentile(send_latencies, 0.50);
    double ls_p95 = percentile(send_latencies, 0.95);
    double ls_p99 = percentile(send_latencies, 0.99);

    double lr_p50 = percentile(recieve_latencies, 0.50);
    double lr_p95 = percentile(recieve_latencies, 0.95);
    double lr_p99 = percentile(recieve_latencies, 0.99);

    std::cout << "Elapsed:" << seconds << " sec\n";

    std::cout << "Throughput: " << rps << " ops/sec\n";

    std::cout << "SEND: \n";

    std::cout << "Avg send latency: "
    << (cum_latency_send / N) << "ms\n";

    std::cout << "p50: " << ls_p50 << " ms\n";
    std::cout << "p95: " << ls_p95 << " ms\n";
    std::cout << "p99: " << ls_p99 << " ms\n";

    std::cout << "RECEIVE: \n";

    std::cout << "Avg receive latency: "
    << (cum_latency_receive / N) << "ms\n";

    std::cout << "p50: " << lr_p50 << " ms\n";
    std::cout << "p95: " << lr_p95 << " ms\n";
    std::cout << "p99: " << lr_p99 << " ms\n";

}