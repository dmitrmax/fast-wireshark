
CC = gcc
CFLAGS = -g -Wall -ansi -pedantic
DFLAGS = -D_POSIX_SOURCE

EXEC = client server

.PHONY: all
all: client server

.PHONY: test
test: all
	sh simple.sh -p 1337

.PHONY: test-nonetwork
test-nonetwork:
	sh simple.sh -f capture.raw

# Specify correct dependencies for each member of EXEC
# (just the .c file)
$(foreach f, $(EXEC), $(eval $(f): $(f).c))

$(EXEC):
	$(CC) $(CFLAGS) $(DFLAGS) -o $(@) $(@).c

.PHONY: clean
clean:
	rm -f $(EXEC)

