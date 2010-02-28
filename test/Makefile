
CC = gcc
CFLAGS = -g -Wall -ansi -pedantic
DFLAGS = -D_POSIX_SOURCE
IFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include
LFLAGS = -lglib-2.0

OBJCC = $(CC) -c $(CFLAGS) $(DFLAGS) $(IFLAGS)
EXECC = $(CC) $(CFLAGS) $(LFLAGS)

EXEC = client server
OBJS = $(EXEC) encode

.PHONY: all
all: client server

.PHONY: test
test: all
	sh simple.sh -p 1337

.PHONY: test-nonetwork
test-nonetwork:
	sh simple.sh -f capture.raw


# All objects have deps
$(foreach f, $(OBJS), $(eval $(f).o: $(f).c $(f).h))

# Each exec 
$(foreach f, $(EXEC), $(eval $(f): $(f).o))

client: encode.o

$(OBJS:%=%.o):
	$(OBJCC) -o $(@) $(@:%.o=%.c)

$(EXEC):
	$(EXECC) -o $(@) $(^)

.PHONY: clean
clean:
	rm -f $(EXEC) $(OBJS:%=%.o)

