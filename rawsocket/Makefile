VERSION_STR="\"0.1\""

CFLAGS=-DVERSION_STR=$(VERSION_STR) -g 
LDFLAGS=-lpthread
C_OBJECTS=client.o 
S_OBJECTS=server.o

all: client server
 
client: $(C_OBJECTS) 
	$(CC) -o $@ $(C_OBJECTS) $(LDFLAGS) 

server: $(S_OBJECTS)
	$(CC) -o $@ $(S_OBJECTS) 

clean: 
	rm -f $(C_OBJECTS) client
	rm -f $(S_OBJECTS) server
