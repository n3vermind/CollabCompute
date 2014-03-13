CC=g++
CFLAGS=--std=c++11 -lboost_system -lboost_filesystem -lpthread
SOURCES=Server.cpp Connection.cpp Identify.cpp sha1.cpp main.cpp Console.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(CFLAGS)

%.o: %.cpp %.hpp 
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJECTS) $(EXECUTABLE)
