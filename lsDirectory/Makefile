CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = main.o fat32.o ls.o readFile.o 
DEPS = fat32.h ls.h readFile.h types.h 
TARGET = lsdir

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJ) $(TARGET)