#ifndef KERNEL_BYPASS_CONFIG_H
#define KERNEL_BYPASS_CONFIG_H

#define pr_fmt(fmt) "kernel-bypass:" KBUILD_MODNAME ": " fmt

#include <linux/moduleparam.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_NAME "kernel-bypass"
#define MODULE_PARAM_PREFIX MODULE_NAME "."

#endif /* KERNEL_BYPASS_CONFIG_H */
