QEMU = qemu-system-i386
QEMUFLAGS = -s -drive format=raw,file=$(OS_FILE),index=0,if=ide, -m 128M \
			-netdev tap,id=mynet0,ifname=tap0,script=no,downscript=no \
			-device rtl8139,netdev=mynet0,mac=de:ad:be:ef:66:69 \
			-monitor stdio \
			-object filter-dump,id=f1,netdev=mynet0,file=build/dump.dat

# -netdev tap,helper=/usr/lib/qemu/qemu-bridge-helper,id=u1,ifname=tap0,script=no,downscript=no -device e1000,netdev=u1,mac=52:55:00:d1:55:01
# -netdev tap,helper=/usr/lib/qemu/qemu-bridge-helper,id=u1 -device rtl8139,netdev=u1,id=simpleos_nic,mac=de:ad:be:ef:66:69 \
  -monitor stdio \
  -object filter-dump,id=f1,netdev=u1,file=dump.dat

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
