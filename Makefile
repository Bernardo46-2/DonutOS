QEMU = qemu-system-x86_64
QEMUFLAGS = -drive format=raw,file=$(OS_FILE),index=0,if=ide, -m 128M -nic user,model=virtio-net-pci,mac=de:ad:be:ef:66:69 -monitor stdio

#-netdev user,id=mynet0 \
-device virtio-net-pci,netdev=mynet0,mac=de:ad:be:ef:66:69 \
-object filter-dump,id=f1,netdev=mynet0,file=$(BUILD_DIR)/net_dump.dat \
#-nic tap,id=mynet0,ifname=tap0,script=no,downscript=no,model=virtio-net-pci

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
