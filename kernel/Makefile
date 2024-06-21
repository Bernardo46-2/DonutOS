CROSS_COMPILER := $(shell command -v /usr/local/i386elfgcc/bin/i386-elf-gcc 2> /dev/null)

ifdef CROSS_COMPILER
	CC = /usr/local/i386elfgcc/bin/i386-elf-gcc
	CFLAGS = -ffreestanding -Iinclude -m32 -g
else
	CC = gcc
	CFLAGS = -ffreestanding -Iinclude -m32 -g -march=i386 -fno-pie -fno-stack-protector -Os
endif

LD = ld
# LDFLAGS = -Ttext 0x8000 --oformat binary -m elf_i386 
LDFLAGS = -Ttext 0x8000 -m elf_i386 

AS = nasm
ASFLAGS = -f elf

BUILD_DIR = ../build/kernel
SRC_DIR = src
KERNEL_DIR = kernel

SRC_FILES = $(wildcard $(SRC_DIR)/*.*)
KERNEL_FILES = $(wildcard $(KERNEL_DIR)/*.*)

LIBC_OBJS = $(wildcard ../build/libc/*.o)
KERNEL_OBJS = \
    $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(filter %.c,$(SRC_FILES))) \
    $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.o, $(filter %.s,$(SRC_FILES))) \
    $(patsubst $(KERNEL_DIR)/%, $(BUILD_DIR)/%.o, $(basename $(KERNEL_FILES)))

FIRST_LINKER_OBJS = $(BUILD_DIR)/kernel_entry.o $(BUILD_DIR)/kernel.o
OTHER_LINKER_OBJS = $(filter-out $(FIRST_LINKER_OBJS), $(KERNEL_OBJS))

FULL_KERNEL = full_kernel.bin

all: setup $(BUILD_DIR)/$(FULL_KERNEL)

$(BUILD_DIR)/$(FULL_KERNEL): $(KERNEL_OBJS)
	$(LD) -o $(BUILD_DIR)/full_kernel.elf $(LDFLAGS) $(FIRST_LINKER_OBJS) $(OTHER_LINKER_OBJS) $(LIBC_OBJS)
	objcopy -O binary $(BUILD_DIR)/full_kernel.elf $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(KERNEL_DIR)/%.s
	$(AS) $< $(ASFLAGS) -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(AS) $< $(ASFLAGS) -o $@

rebuild: clean all

setup:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all rebuild setup clean
