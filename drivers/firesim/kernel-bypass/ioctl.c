#include "config.h"
#include "ioctl.h"

#include <linux/cpu.h>
#include <linux/kernel.h>

#include <asm/csr.h>

/* Hacky way at getting at the do_page_fault handler in traps.c. The handler
 * does not have a prototype defined in a shared header (because you are never
 * supposed to call that function yourself), so we just tell the compiler the
 * function is extern and that we will punt the problem off to the linker. */
extern void do_page_fault(struct pt_regs *regs);

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
  pr_debug("New STARGET: " REG_FMT "\n", csr_read(CSR_STARGET));
  return 0;
}

/** Enable/Disable a particular pattern of traps. */
int ioctl_delegate_traps(struct delegate_config_t trap_setup)
{
  pr_info("Enable/Disable: %s\n", trap_setup.en_flag == 1 ? "Enable" : "Disable");
  pr_info("Trap Delegation Mask: 0x" REG_FMT "\n", trap_setup.trap_mask);
  pr_debug("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_STATUS));

  struct pt_regs *regs = task_pt_regs(current);
  pr_debug("pt_regs->status: " REG_FMT "\n", regs->status);
  switch (trap_setup.en_flag) {
  case 0:
    pr_info("Clearing/Disabling SEDELEG\n");
    csr_clear(CSR_SEDELEG, trap_setup.trap_mask);
    /* XXX: DISABLE UIE! */
    /* regs->status &= 0xFFFFFFFFFFFFFFFE; */
    regs->status &= ~SR_UIE;
    break;
  case 1:
    pr_info("Setting/Enabling SEDELEG\n");
    csr_set(CSR_SEDELEG, trap_setup.trap_mask);
    /* XXX: ENABLE UIE! */
    /* regs->status |= 0x0000000000000001; */
    regs->status |= SR_UIE;
    break;
  default:
    pr_alert("Invalid trap delegation enable option! %ud is unsupported! Doing nothing\n",
             trap_setup.en_flag);
    break;
  }

  unsigned long new_sedeleg = csr_read(CSR_SEDELEG);
  pr_debug("New SEDELEG: " REG_FMT "\n", new_sedeleg);
  pr_debug("New SSTATUS: " REG_FMT "\n", csr_read(CSR_STATUS));
  pr_debug("New pt_regs->status: " REG_FMT "\n", regs->status);

  return 0;
}

/*
 * User program that has enabled KBEs for page faults is requesting the kernel
 * to handle some part of the page fault.
 */
int ioctl_handle_kbe_page_fault(struct kbe_page_fault_t fault)
{
    struct pt_regs regs = {0};
    /* Build a somewhat fake pt_regs and pass it off to the normal page fault
     * handler.
     * In particular, we need to set the CAUSE to the right kind of page fault,
     * make the system believe we are coming from user-space, and install the
     * bad address we got. */
    pr_info("Handling KBE Page fault request for user vaddr 0x" REG_FMT "\n",
	    fault.fault_vaddr);

    pr_info("UCAUSE: 0x" REG_FMT "\n", csr_read(CSR_UCAUSE));
    pr_info("UTVAL: 0x" REG_FMT "\n", csr_read(CSR_UTVAL));

    switch(fault.kind) {
    case CODE:
	regs.cause = EXC_INST_PAGE_FAULT;
	pr_info("Handling Code/INSTruction page fault\n");
	break;
    case LOAD:
	regs.cause = EXC_LOAD_PAGE_FAULT;
	pr_info("Handling LOAD page fault request\n");
	break;
    case STORE:
	regs.cause = EXC_STORE_PAGE_FAULT;
	pr_info("Handling STORE page fault request\n");
	break;
    default:
	die(&regs, "Unknown type of KBE page fault request!");
	break;
    }
    regs.status = regs.status & SR_UPP;
    regs.badaddr = fault.fault_vaddr;

    pr_info("Handling page fault by calling do_page_fault");
    do_page_fault(&regs);

    return 0;
}

void ioctl_handle_time(struct kbe_ioctl_time_t* time)
{
    time->start_ioctl = csr_read(CSR_CYCLE);
    pr_debug("Handling time measurement ioctl\n");
    time->hit_kernel = csr_read(CSR_USSCRATCH);
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
  pr_info("SALREADY_HANDLING: 0x" REG_FMT "\n", csr_read(CSR_SALREADY_HANDLING));
  pr_info("UALREADY_HANDLING: 0x" REG_FMT "\n", csr_read(CSR_UALREADY_HANDLING));
  pr_info("USCRATCH: 0x" REG_FMT "\n", csr_read(CSR_USCRATCH));
  pr_info("UEPC: 0x" REG_FMT "\n", csr_read(CSR_UEPC));
  pr_info("UCAUSE: 0x" REG_FMT "\n", csr_read(CSR_UCAUSE));
  pr_info("UTVAL: 0x" REG_FMT "\n", csr_read(CSR_UTVAL));
  return 0;
}
