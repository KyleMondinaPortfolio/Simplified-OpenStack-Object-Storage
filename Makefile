CC=g++
CFLAGS=-I.
CPPFLAGS=-std=c++11
LIBS=-lssl -lcrypto  # OpenSSL libraries
TARGETS=Test Server Client
OBJECTS=MachineList.o PartitionMap.o ObjectManager.o Server.o Client.o server.o main.o

all: $(TARGETS)

Test: main.o MachineList.o PartitionMap.o ObjectManager.o Server.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

Server: server.o Server.o Client.o ObjectManager.o MachineList.o PartitionMap.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

Client: client.o Client.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGETS) *.o
