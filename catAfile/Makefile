CC = gcc
CFLAGS = -Wall -Wextra -O2
OBJ = main.o fat32.o dir.o readFile.o
DEPS = fat32.h dir.h readFile.h types.h 
TARGET = catF

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

.PHONY: clean

clean:
	rm -f $(OBJ) $(TARGET)
