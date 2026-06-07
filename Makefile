CXX := g++
MODE ?= debug
BUILD := build
HOST ?= 127.0.0.1

CXXFLAGS := -std=c++17 -Wall -Wextra -Iinclude
TESTFLAGS := -lgtest -lgtest_main -lpthread

SERVER_SRCS = src/server.cpp \
				src/storage/hashtable.cpp \
				src/storage/heap.cpp

COMMON_SRCS = src/logs/log.cpp

ifeq ($(MODE), debug)
    CXXFLAGS += -g -O0 -DLOG_LEVEL=0

else ifeq ($(MODE), release)
	CXXFLAGS += -O2 -DNDEBUG -DLOG_LEVEL=2

else
    $(error Unknown MODE '$(MODE)'. Use MODE=debug or MODE=release)

endif

.PHONY: all clean run_server run_client tests run_tests

all: $(BUILD)/client $(BUILD)/server
tests: $(BUILD)/tests

$(BUILD)/client: src/client.cpp $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: $(SERVER_SRCS) $(COMMON_SRCS)
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/tests: tests/unit/test_heap.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ $(TESTFLAGS) -o $@

run_tests: tests 
			./$(BUILD)/tests

run_server: all 
			./$(BUILD)/server 

run_server_prod: all
				./$(BUILD)/server &

run_client: all 
			./$(BUILD)/client --host $(HOST) 

clean:
	rm -rf $(BUILD)