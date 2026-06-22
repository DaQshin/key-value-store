CXX := g++
MODE ?= debug
BUILD := build

CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
TESTFLAGS := -lCatch2Main -lCatch2
BENCHFLAGS := -O2 -DNDEBUG -DLOG_LEVEL=0

SERVER_SRCS = src/server.cpp \
				src/storage/hashtable.cpp \
				src/storage/heap.cpp

COMMON_SRCS = src/utils/log.cpp

BENCH_SRCS = $(wildcard bench/*.cpp)
TEST_SRCS = $(wildcard tests/*.cpp)

ifeq ($(MODE), debug)
    CXXFLAGS += -g -O0 -DLOG_LEVEL=0

else ifeq ($(MODE), release)
	CXXFLAGS += -O2 -DNDEBUG -DLOG_LEVEL=2

else
    $(error Unknown MODE '$(MODE)'. Use MODE=debug or MODE=release)

endif

.PHONY: all clean run_server run_client tests run_tests bench run_bench run_server_prod

all: $(BUILD)/client $(BUILD)/server
tests: $(BUILD)/tests
bench: $(BUILD)/bench

$(BUILD)/client: src/client.cpp $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: $(SERVER_SRCS) $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/tests: tests/unit/test_heap.cpp src/storage/heap.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ $(TESTFLAGS) -o $@

$(BUILD)/bench: $(BENCH_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@
	
run_bench: bench 
			./$(BUILD)/bench

run_tests: tests 
			./$(BUILD)/tests

run_server: all 
			./$(BUILD)/server 

run_server_prod: all
				./$(BUILD)/server &

run_client: all 
			./$(BUILD)/client

clean:
	rm -rf $(BUILD)