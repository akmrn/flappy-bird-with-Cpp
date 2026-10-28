CXX = g++

CXXFLAGS = -Iinclude -Wall -std=c++17

LIBS = -lSDL3 -lSDL3_image -lSDL3_ttf

TARGET = flappyBird.exe

# Added src/Game.cpp to the source list
SRCS = main.cpp src/Game.cpp src/Pipe.cpp src/ScoreSystem.cpp src/player.cpp src/Ground.cpp

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	del /Q $(TARGET)
