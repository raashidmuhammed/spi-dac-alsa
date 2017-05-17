obj-m      := spi-dac-alsa.o
KERNEL_DIR ?= "/home/user/meetups/wifi-router/linux-4.4.39"
CCPREFIX   := "arm-none-linux-gnueabi-"

.PHONY: all clean distclean

all:
	make -C $(KERNEL_DIR) M=${PWD} ARCH=arm CROSS_COMPILE=$(CCPREFIX) modules

clean:
	make -C $(KERNEL_DIR) M=${PWD} ARCH=arm CROSS_COMPILE=$(CCPREFIX) clean

distclean: clean
	rm -f *~
