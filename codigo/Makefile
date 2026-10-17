CXX = g++
CXXFLAGS = -std=c++11 -Wall -O3
TARGET = simulador_cache
SRC = Proyecto.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)