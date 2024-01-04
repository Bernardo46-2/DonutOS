CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
CFLAGS = -ffreestanding -Iinclude -m32 -g

SRC_DIR = src
BUILD_DIR = ../build/libc
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
LIBC_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES))

all: setup $(LIBC_OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

rebuild: clean all

setup:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all rebuild setup clean