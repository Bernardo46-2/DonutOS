QEMU = qemu-system-x86_64
QEMUFLAGS = -drive format=raw,file=$(OS_FILE),index=0,if=ide, -m 128M -netdev user,id=usernet -device e1000,netdev=usernet

SUB_DIRS = libc boot kernel

BUILD_DIR = build
OS_FILE = $(BUILD_DIR)/OS.bin

BOOT = build/boot/boot.bin
FULL_KERNEL = build/kernel/full_kernel.bin
ZEROES = build/boot/zeroes.bin

all: setup
	$(foreach dir, $(SUB_DIRS), $(MAKE) all -C $(dir);)
	cat $(BOOT) $(FULL_KERNEL) $(ZEROES) > $(OS_FILE)

run: setup
	$(foreach dir, $(SUB_DIRS), $(MAKE) rebuild -C $(dir);)
	cat $(BOOT) $(FULL_KERNEL) $(ZEROES) > $(OS_FILE)
	$(QEMU) $(QEMUFLAGS)

setup:
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run setup clean
