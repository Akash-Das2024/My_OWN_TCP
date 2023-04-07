CC = gcc
AR = ar
CFLAGS = -Wall -Werror

all: libmsocket.a

mysocket.o: mysocket.c mysocket.h
	$(CC) $(CFLAGS) -c $< -o $@

libmsocket.a: mysocket.o
	$(AR) rcs $@ $<

clean:
	rm -f mysocket.o libmsocket.a
