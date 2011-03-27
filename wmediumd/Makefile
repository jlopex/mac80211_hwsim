VERSION_STR="\"0.1\""

CFLAGS=-DVERSION_STR=$(VERSION_STR) -g 
LDFLAGS=-lnl -lnl-genl -L/usr/local/lib  
OBJECTS=nlclient.o probability.o
 
wmediumd: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) 
 
clean: 
	rm -f $(OBJECTS) wmediumd
