CC=g++
CFLAGS=-I.
CPPFLAGS=-std=c++11
LIBS=-lssl -lcrypto  # OpenSSL libraries
TARGETS=Test
OBJECTS=MachineList.o PartitionMap.o main.o

all: $(TARGETS)

Test: main.o MachineList.o PartitionMap.o
	$(CC) $(CPPFLAGS) -o $@ $^ $(LIBS)

%.o: %.cpp
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGETS) *.o
