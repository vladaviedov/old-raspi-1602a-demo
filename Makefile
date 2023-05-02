CC = gcc
CFLAGS = -Wall -g
LIBS = -li2c

all: input scroll

input: input1.c
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

scroll: scroll.c
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

clean:
	rm input
