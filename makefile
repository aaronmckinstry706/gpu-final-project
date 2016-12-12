CC = g++
CFLAGS = 
OBJS = encode.o encode_library.o
PROG = encode

all: $(PROG)

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o encode

encode.o: encode.cpp encode_library.h
encode_library.o: encode_library.h

clean:
	rm -f *~ *.o $(PROG) core a.out
