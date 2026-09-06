#include "config.h"
#include "ioctl.h"

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/sched/debug.h>

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
int ioctl_install_handler_address(struct pt_regs *regs,
				  unsigned long target_addr)
{
  pr_info("Setting handler target to addr 0x" REG_FMT "\n", target_addr);
  regs->starget = target_addr;
  pr_debug("New STARGET: " REG_FMT "\n", regs->starget);
  return 0;
}

/** Enable/Disable a particular pattern of traps. */
int ioctl_delegate_traps(struct pt_regs *regs, struct delegate_config_t trap_setup)
{
  pr_info("Enable/Disable: %s\n", trap_setup.en_flag == 1 ? "Enable" : "Disable");
  pr_info("Trap Delegation Mask: 0x" REG_FMT "\n",
	  (unsigned long)trap_setup.trap_mask);
  pr_debug("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_STATUS));

  pr_debug("pt_regs->status: " REG_FMT "\n", regs->status);
  switch (trap_setup.en_flag) {
  case 0:
    pr_info("Clearing/Disabling SEDELEG\n");
    regs->sedeleg &= ~trap_setup.trap_mask;
    /* XXX: DISABLE UIE! */
    regs->status &= ~SR_UIE;
    break;
  case 1:
    pr_info("Setting/Enabling SEDELEG\n");
    regs->sedeleg |= trap_setup.trap_mask;
    /* XXX: ENABLE UIE! */
    regs->status |= SR_UIE;
    break;
  default:
    pr_alert("Invalid trap delegation enable option! %ud is unsupported! Doing nothing\n",
             trap_setup.en_flag);
    break;
  }

  pr_debug("New SEDELEG: " REG_FMT "\n", regs->sedeleg);
  pr_debug("New SSTATUS: " REG_FMT "\n", csr_read(CSR_STATUS));
  pr_debug("New pt_regs->status: 0x" REG_FMT "\n", regs->status);
  pr_info("New SEDELEG: 0x" REG_FMT "\n", csr_read(CSR_SEDELEG));
  pr_info("New pt_regs->SEDELEG: 0x" REG_FMT "\n", regs->sedeleg);

  return 0;
}

/*
 * User program that has enabled KBEs for page faults is requesting the kernel
 * to handle some part of the page fault.
 */
int ioctl_handle_kbe_page_fault(struct pt_regs *real_regs,
				struct kbe_page_fault_t fault)
{
    struct pt_regs fake_regs = {0};

    /* Force regs to be clean and zero, just in case the stack has garbage at
     * the location where regs ended up. */
    memcpy(&fake_regs, real_regs, sizeof(struct pt_regs));

    /* Build a somewhat fake pt_regs and pass it off to the normal page fault
     * handler.
     * In particular, we need to set the CAUSE to the right kind of page fault,
     * make the system believe we are coming from user-space, and install the
     * bad address we got. */
    pr_info("Handling KBE Page fault request for user vaddr 0x" REG_FMT "\n",
	    (long unsigned)fault.fault_vaddr);

    pr_info("SEPC: 0x" REG_FMT " (where we called ioctl from)\n", csr_read(CSR_SEPC));
    pr_info("UCAUSE: 0x" REG_FMT "\n", csr_read(CSR_UCAUSE));
    pr_info("UEPC: 0x" REG_FMT " (insn that made KBE'd page fault)\n", csr_read(CSR_UEPC));
    pr_info("UTVAL: 0x" REG_FMT "\n", csr_read(CSR_UTVAL));
    pr_info("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_STATUS));

    pr_info("What the user told us:\n");
    pr_info("fault.kind = %d\n", fault.kind);
    pr_info("fault.epc = 0x" REG_FMT "\n",
	    (unsigned long)fault.epc);
    pr_info("fault.fault_vaddr = 0x" REG_FMT "\n",
	    (long unsigned)fault.fault_vaddr);

    const long unsigned kern_addrs_mask = 0xffffff8000000000UL;
    if ((fault.epc & kern_addrs_mask) != 0) {
	pr_warn("Trying to handle a page fault on 0x" REG_FMT " for insn @ 0x" REG_FMT " located in KERNEL SPACE!\n",
		(long unsigned)fault.epc, (long unsigned)fault.fault_vaddr);
    }

    if ((fault.fault_vaddr & kern_addrs_mask) != 0) {
	pr_warn("Trying to handle a page fault for KERNEL PAGE at 0x" REG_FMT " for insn @ 0x" REG_FMT "\n",
		(long unsigned)fault.fault_vaddr, (long unsigned)fault.epc);
    }

    if (fault.fault_vaddr == 0) {
	pr_warn("Given faulting address of NULL! Inst causing fault 0x" REG_FMT "\n",
		(long unsigned)fault.epc);
    }

    fake_regs.epc = fault.epc;
    fake_regs.badaddr = fault.fault_vaddr;

    switch(fault.kind) {
    case CODE:
	fake_regs.cause = EXC_INST_PAGE_FAULT;
	pr_info("Handling Code/INSTruction page fault\n");
	break;
    case LOAD:
	fake_regs.cause = EXC_LOAD_PAGE_FAULT;
	pr_info("Handling LOAD page fault request\n");
	break;
    case STORE:
	fake_regs.cause = EXC_STORE_PAGE_FAULT;
	pr_info("Handling STORE page fault request\n");
	break;
    default:
	die(&fake_regs, "Unknown type of KBE page fault request!");
	break;
    }

    // Explicitly denote that the previous privilege mode was user-mode.
    fake_regs.status &= SR_UPP;

    show_regs(&fake_regs);

    pr_info("Handling page fault by calling do_page_fault\n");
    do_page_fault(&fake_regs);
    pr_info("do_page_fault completed!\n");

    pr_info("SEPC: 0x" REG_FMT " (should be where we called ioctl from)\n", csr_read(CSR_SEPC));
    pr_info("UCAUSE: 0x" REG_FMT "\n", csr_read(CSR_UCAUSE));
    pr_info("UEPC: 0x" REG_FMT " (insn that made KBE'd page fault)\n", csr_read(CSR_UEPC));
    pr_info("UTVAL: 0x" REG_FMT "\n", csr_read(CSR_UTVAL));
    pr_info("SSTATUS: 0x" REG_FMT "\n", csr_read(CSR_STATUS));

    show_regs(&fake_regs);
    pr_info("Exiting KBE page fault request ioctl handler\n");
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
