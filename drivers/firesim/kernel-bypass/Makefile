obj-m += kernel-bypass.o

kernel-bypass-y += kernel-bypass-main.o chardev.o \
	           ioctl.o

ccflags-y := -D__KERNEL__
# CFLAGS_kernel_bypass.o := -D__KERNEL__
