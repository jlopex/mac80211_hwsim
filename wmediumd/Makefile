VERSION_STR="\"0.1\""

PKG_CONFIG ?= pkg-config
NL2FOUND := $(shell $(PKG_CONFIG) --atleast-version=2 libnl-2.0 && echo Y)
NL3FOUND := $(shell $(PKG_CONFIG) --atleast-version=3 libnl-3.0 && echo Y)

ifeq ($(NL2FOUND),Y)
LDFLAGS += -lnl -lnl-genl
NLLIBNAME= libnl-2
endif

ifeq ($(NL3FOUND),Y)
CFLAGS += -I/usr/local/include/libnl3/
LDFLAGS += -lnl-3 -lnl-genl-3
NLLIBNAME= libnl-3
endif

ifeq ($(NLLIBNAME),)
$(error Cannot find development files for any supported version of libnl)
endif

CFLAGS+=-DVERSION_STR=$(VERSION_STR) -g
LDFLAGS+=-lconfig 
OBJECTS=wmediumd.o probability.o mac_address.o config.o

all: version_check wmediumd 

version_check:
ifeq ($(NL3FOUND),Y)
else
ifeq ($(NL2FOUND),Y)
else
	$(error No libnl found)
endif
endif

wmediumd: $(OBJECTS) 
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS) 
 
clean: 
	rm -f $(OBJECTS) wmediumd
