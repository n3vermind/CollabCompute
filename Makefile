CC=g++
CFLAGS=--std=c++11 -lboost_system

SOURCES=Server.cpp Connection.cpp Identify.cpp sha1.cpp main.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
