#include "config.h"
#include "chardev.h"

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

#include <asm/io.h>
#include <asm/csr.h>
#include <asm/ptrace.h>

static int __init kernel_bypass_init(void)
{
  pr_info("Starting Pipelined Delegation module\n");

  int rc = 0;

  rc = create_char_devs();

  struct pt_regs *regs = task_pt_regs(current);
  pr_info("a0: 0x" REG_FMT "\n", regs->a0);

  pr_info("SSTATUS: " REG_FMT "\n", csr_read(CSR_SSTATUS));
  pr_info("STARGET: " REG_FMT "\n", csr_read(CSR_STARGET));
  pr_info("SEDELEG: " REG_FMT "\n", csr_read(CSR_SEDELEG));
  pr_info("SIDELEG: " REG_FMT "\n", csr_read(CSR_SIDELEG));

  return 0;
}

static void __exit kernel_bypass_exit(void)
{
  pr_info("Stopping Pipelined Delegation module\n");

  pr_info("Clearing all pipelined delegation CSRs to return to default behavior\n");
  csr_set(CSR_STARGET, 0);
  csr_set(CSR_SEDELEG, 0);
  csr_set(CSR_SIDELEG, 0);
  pr_info("STARGET: " REG_FMT "\n", csr_read(CSR_STARGET));
  pr_info("SEDELEG: " REG_FMT "\n", csr_read(CSR_SEDELEG));
  pr_info("SIDELEG: " REG_FMT "\n", csr_read(CSR_SIDELEG));

  int rc = 0;
  rc = destroy_char_devs();
}

module_init(kernel_bypass_init);
module_exit(kernel_bypass_exit);
MODULE_VERSION("0.0");
MODULE_DESCRIPTION("Work with Pipelined Delegation");
MODULE_LICENSE("UNKNOWN");
