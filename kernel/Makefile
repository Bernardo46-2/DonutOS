CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
CFLAGS = -ffreestanding -Iinclude -m32 -g

LD = /usr/local/i386elfgcc/bin/i386-elf-ld
LDFLAGS = -Ttext 0x8000 --oformat binary

AS = nasm
ASFLAGS = -f elf

BUILD_DIR = ../build/kernel
SRC_DIR = src
KERNEL_DIR = kernel

SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
KERNEL_FILES = $(wildcard $(KERNEL_DIR)/*.*)

LIBC_OBJS = $(wildcard ../build/libc/*.o)
KERNEL_OBJS = \
	$(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRC_FILES)) \
	$(patsubst $(KERNEL_DIR)/%, $(BUILD_DIR)/%.o, $(basename $(KERNEL_FILES)))

FIRST_LINKER_OBJS = $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o
OTHER_LINKER_OBJS = $(filter-out $(FIRST_LINKER_OBJS), $(KERNEL_OBJS))

FULL_KERNEL = full_kernel.bin

all: setup $(BUILD_DIR)/$(FULL_KERNEL)

$(BUILD_DIR)/$(FULL_KERNEL): $(KERNEL_OBJS)
	$(LD) -o $@ $(LDFLAGS) $(FIRST_LINKER_OBJS) $(OTHER_LINKER_OBJS) $(LIBC_OBJS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.s
	$(AS) $< $(ASFLAGS) -o $@

rebuild: clean all

setup:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all rebuild setup clean
