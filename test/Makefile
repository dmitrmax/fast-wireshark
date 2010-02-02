
CC = gcc
CFLAGS = -g -Wall -ansi -pedantic
DFLAGS = -D_POSIX_SOURCE

EXEC = client server

.PHONY: default
default: client server

# Specify correct dependencies for each member of EXEC
# (just the .c file)
$(foreach f, $(EXEC), $(eval $(f): $(f).c))

$(EXEC):
	$(CC) $(CFLAGS) $(DFLAGS) -o $(@) $(@).c

.PHONY: clean
clean:
	rm -f $(EXEC)

