SHELL=/bin/sh
MAKE = make
SUBDIRS ?= mac80211_hwsim rawsocket wmediumd

all:

	@for i in $(SUBDIRS); do \
	echo "make all in $$i..."; \
	(cd $$i; $(MAKE) all); done

clean:

	@for i in $(SUBDIRS); do \
	echo "Clearing in $$i..."; \
	(cd $$i; $(MAKE) clean); done

 
