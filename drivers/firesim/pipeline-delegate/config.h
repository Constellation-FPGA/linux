#ifndef PIPELINED_DELEGATE_CONFIG_H
#define PIPELINED_DELEGATE_CONFIG_H

#define pr_fmt(fmt) "pipelined-delegate:" KBUILD_MODNAME ": " fmt

#include <linux/moduleparam.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_NAME "pipelined-delegate"
#define MODULE_PARAM_PREFIX MODULE_NAME "."

#endif /* PIPELINED_DELEGATE_CONFIG_H */
