CXX = g++

CXXFLAGS = -Iinclude -Wall -std=c++17

LIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

TARGET = flappyBird.exe

SRCS = main.cpp src/Pipe.cpp src/ScoreSystem.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	del /Q $(TARGET)