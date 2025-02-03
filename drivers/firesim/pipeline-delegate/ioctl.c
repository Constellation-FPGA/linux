#include "config.h"
#include "ioctl.h"

#include <linux/cpu.h>
#include <linux/kernel.h>

#include <asm/csr.h>

int ioctl_install_handler_address(unsigned long target_addr)
{
  pr_info("Setting handler target to addr 0x%lX\n", target_addr);
  return 0;
}

int ioctl_delegate_traps(struct delegate_config_t trap_setup)
{
  pr_info("Enable/Disable: %s\n", trap_setup.en_flag == 1 ? "Enable" : "Disable");
  pr_info("Trap Delegation Mask: 0x%lX\n", trap_setup.trap_mask);
  return 0;
}

/** Dump the values of the pipelined delegation CSRs.
 *
 * Mostly useful for checking the status of the hart when you are running and
 * debugging in Firesim. */
int ioctl_csr_status(void)
{
  pr_info("STARGET: 0x" REG_FMT "\n", csr_read(CSR_STARGET));
  pr_info("SEDELEG: 0x" REG_FMT "\n", csr_read(CSR_SEDELEG));
  pr_info("SIDELEG: 0x" REG_FMT "\n", csr_read(CSR_SIDELEG));
  pr_info("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_SSTATUS));
  pr_info("UEPC: 0x" REG_FMT "\n", csr_read(CSR_UEPC));
  return 0;
}
