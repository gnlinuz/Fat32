CC = gcc
CFLAGS = -Wall -Wextra -std=c11
OBJS = main.o fat32.o cd.o rwFile.o

catFile: $(OBJS)
	$(CC) $(CFLAGS) -o cdir $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o cdir