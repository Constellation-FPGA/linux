#ifndef PIPELINED_DELEGATE_CHARDEV_H
#define PIPELINED_DELEGATE_CHARDEV_H

#include "config.h"

#include <linux/cdev.h>
#include <linux/fs.h>

int create_char_devs(void);
int destroy_char_devs(void);

#endif /* PIPELINED_DELEGATE_CHARDEV_H */
