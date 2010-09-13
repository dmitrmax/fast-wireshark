
CC = gcc
CFLAGS = -g -Wall -ansi -pedantic
DFLAGS = -D_POSIX_SOURCE
IFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
LFLAGS = -lglib-2.0

OBJCC = $(CC) -c $(CFLAGS) $(DFLAGS) $(IFLAGS)
EXECC = $(CC) $(CFLAGS) $(LFLAGS)

EXEC = bin/client bin/server

.PHONY: all
all: $(EXEC)

%.o: %.c %.h
	$(OBJCC) -o $@ $<

bin/client: client/client.o client/encode.o
	$(EXECC) -o $@ $^

bin/server: server/server.o
	$(EXECC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(EXEC) */*.o

