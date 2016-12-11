CC = gcc
CFLAGS = -std=c99
OBJS = encode.o encode_library.o
PROG = encode

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o encode

encode.o: encode.c encode_library.h
encode_library.o: encode_library.h

clean:
	rm -f *~ *.o $(PROG) core a.out
