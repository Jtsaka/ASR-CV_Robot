CXX = g++
CXXFLAGS = -std=c++17 -Wall
TARGET = asr
SRC = asr.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)
