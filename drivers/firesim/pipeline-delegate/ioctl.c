#include "config.h"
#include "ioctl.h"

#include <linux/cpu.h>
#include <linux/kernel.h>

#include <asm/csr.h>

/** Install a new virtual address to be used by pipelined exceptions.
 *
 * The kernel does not need to do anything. The delegation is done entirely
 * within hardware, bypassing the kernel entirely. HOWEVER, the pipelined
 * handler STILL relies on the kernel to provide virtual address translation.
 * How this will play together with things like page faults, I don't know yet. */
int ioctl_install_handler_address(unsigned long target_addr)
{
  pr_info("Setting handler target to addr 0x" REG_FMT "\n", target_addr);
  csr_write(CSR_STARGET, target_addr);
  pr_info("New STARGET: " REG_FMT "\n", csr_read(CSR_STARGET));
  return 0;
}

/** Enable/Disable a particular pattern of traps. */
int ioctl_delegate_traps(struct delegate_config_t trap_setup)
{
  pr_info("Enable/Disable: %s\n", trap_setup.en_flag == 1 ? "Enable" : "Disable");
  pr_info("Trap Delegation Mask: 0x" REG_FMT "\n", trap_setup.trap_mask);
  pr_info("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_STATUS));

  switch (trap_setup.en_flag) {
  case 0:
    pr_info("Clearing/Disabling SEDELEG\n");
    csr_clear(CSR_SEDELEG, trap_setup.trap_mask);
    break;
  case 1:
    pr_info("Setting/Enabling SEDELEG\n");
    csr_set(CSR_SEDELEG, trap_setup.trap_mask);
    break;
  default:
    pr_alert("Invalid trap delegation enable option! %ud is unsupported! Doing nothing\n",
             trap_setup.en_flag);
    break;
  }

  unsigned long new_sedeleg = csr_read(CSR_SEDELEG);
  pr_info("New SEDELEG: " REG_FMT "\n", new_sedeleg);

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
