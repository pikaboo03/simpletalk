CC=gcc
CFLAGS=-w -std=c99
PROG=s-talk
OBJS= simp.o list.o

PTHREADFLAGS=-pthread

s-talk: $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(PTHREADFLAGS) $(OBJS)

simp.o: simp.c
	$(CC) $(CFLAGS) -c simp.c

clean:
	rm *.o
