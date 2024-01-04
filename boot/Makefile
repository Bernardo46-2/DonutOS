AS = nasm
ASFLAGS = -f bin

BUILD_DIR = ../build/boot
SRC_FILES = $(wildcard *.s)
BUILD_OBJS = $(patsubst %.s, $(BUILD_DIR)/%.bin, $(SRC_FILES))

all: setup $(BUILD_OBJS)

$(BUILD_DIR)/%.bin: %.s
	$(AS) $< $(ASFLAGS) -o $@

rebuild: clean all

setup:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all rebuild setup clean
