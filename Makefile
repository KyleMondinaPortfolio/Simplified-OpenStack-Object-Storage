CC=g++
CFLAGS=-I.
CPPFLAGS=-std=c++11
LIBS=-lssl -lcrypto  # OpenSSL libraries
TARGETS=Test Server Client
OBJECTS=SCPFunctions.o MachineList.o PartitionMap.o ObjectManager.o Server.o Client.o main.o

all: $(TARGETS)

Test: main.o MachineList.o PartitionMap.o ObjectManager.o Server.o SCPFunctions.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

Server: server.o Server.o Client.o ObjectManager.o MachineList.o PartitionMap.o SCPFunctions.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

Client: client.o Client.o SCPFunctions.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGETS) *.o
