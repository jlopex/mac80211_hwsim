VERSION_STR="\"0.1\""

CFLAGS+=-DVERSION_STR=$(VERSION_STR) -g
LDFLAGS+=-lnl -lnl-genl -lconfig -L/usr/local/lib
OBJECTS=wmediumd.o probability.o mac_address.o config.o

all: wmediumd 
 
wmediumd: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) 
 
clean: 
	rm -f $(OBJECTS) wmediumd
