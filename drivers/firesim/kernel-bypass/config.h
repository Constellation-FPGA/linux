#ifndef KERNEL_BYPASS_CONFIG_H
#define KERNEL_BYPASS_CONFIG_H

#define pr_fmt(fmt) "kernel-bypass:" KBUILD_MODNAME ": " fmt

#include <linux/moduleparam.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_NAME "kernel-bypass"
#define MODULE_PARAM_PREFIX MODULE_NAME "."

/* Taken from arch/riscv/include/asm/csr.h */
#define SR_UIE    _AC(0x00000001, UL) /* User Interrupt Enable */
#define SR_UPP		_AC(0x00000000, UL) /* Previously User */

/* Pipelined Delegation CSRs */
#define CSR_SEDELEG 0x102
#define CSR_SIDELEG 0x103
#define CSR_STARGET 0x5c0
#define CSR_USCRATCH 0x840
#define CSR_UEPC    0x841
#define CSR_UCAUSE    0x842
#define CSR_UTVAL    0x843
#define CSR_USSCRATCH 0x844
#define CSR_SALREADY_HANDLING 0x5C1
#define CSR_UALREADY_HANDLING 0x845

#endif /* KERNEL_BYPASS_CONFIG_H */
