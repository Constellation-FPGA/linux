#ifndef KERNEL_BYPASS_CHARDEV_H
#define KERNEL_BYPASS_CHARDEV_H

#include "config.h"

#include <linux/cdev.h>
#include <linux/fs.h>

int create_char_devs(void);
int destroy_char_devs(void);

#endif /* KERNEL_BYPASS_CHARDEV_H */
