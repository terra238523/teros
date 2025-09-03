# Makefile for TerOS

CC = gcc
CFLAGS = -static -O2
ROOTFS = rootfs
INITRAMFS = initramfs.cpio.gz
INIT_BINARY = init

.PHONY: all clean

all: $(INITRAMFS)

# Build the initramfs image
$(INITRAMFS): $(INIT_BINARY).c
	@echo "Compiling init.c..."
	$(CC) $(CFLAGS) -o $(INIT_BINARY) $(INIT_BINARY).c
	@echo "Preparing rootfs..."
	mkdir -p $(ROOTFS)/sbin
	cp $(INIT_BINARY) $(ROOTFS)/sbin/
	chmod +x $(ROOTFS)/sbin/$(INIT_BINARY)
	# Create /init entry point (required by Linux 6.x+)
	cp $(ROOTFS)/sbin/$(INIT_BINARY) $(ROOTFS)/init
	chmod +x $(ROOTFS)/init
	@echo "Packing initramfs..."
	cd $(ROOTFS) && find . | cpio -o --format=newc | gzip > ../$(INITRAMFS)
	@echo "Done! Generated $(INITRAMFS)"

clean:
	@echo "Cleaning..."
	rm -f $(INIT_BINARY) $(INITRAMFS)
	cd $(ROOTFS) && rm -f init
