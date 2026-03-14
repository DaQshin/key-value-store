CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -Iinclude

BUILD := build

.PHONY: all clean run

all: $(BUILD)/client $(BUILD)/server

$(BUILD)/client: src/client.cpp src/logging/log.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BUILD)/server: src/server.cpp src/logging/log.cpp
	mkdir -p $(BUILD)
	$(CXX) $(CXXFLAGS) $^ -o $@

run_server: all 
			./$(BUILD)/server

run_client: all 
			./$(BUILD)/client

clean:
	rm -rf $(BUILD)