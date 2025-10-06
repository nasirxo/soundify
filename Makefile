# Simple Makefile for direct compilation with g++
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O3 -march=native -Iinclude
LDFLAGS = -lm

TARGET = audio_encoder_decoder
SRC_DIR = src
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -f *.wav *.decoded

run_example: $(TARGET)
	@echo "Running example encoding..."
	./$(TARGET) encode examples/test.txt output.wav
	@echo "Running example decoding..."
	./$(TARGET) decode output.wav ./
