#ifndef KERNEL_BYPASS_IOCTL_H
#define KERNEL_BYPASS_IOCTL_H

#ifndef __KERNEL__
typedef unsigned int  __u32;
typedef unsigned long __u64;
#endif

/* The magic 'F' has MANY drivers. Some other sequence numbers (the second param)
 * are taken. I use between 0x30 and 0x80 to give myself room to experiment.
 * To define a new ioctl number, I recommend you use one of the 4 macros below:
 * _IO(magic, number) - No inputs/outputs
 * _IOR(magic, number, input_data_type) - ioctl with input
 * _IOW(magic, number, output_data_type) - ioctl with output
 * _IORW(magic, number, in_out_data_type) - ioctl with input and output
 * ALL ioctls THAT TAKE DATATYPE PARAMETERS ONLY TAKE THE PARAMETER!!
 * i.e. _IOR(magic, number, struct struct_name), NOT
 *      _IOR(magic, number, sizeof(struct struct_name)).
 * Note that the struct is limited to a maximum of 16KiB (14 address bits) */
#define KBE_IOCTL_MAGIC 'F'

/** The basics we need to get exceptions bypassing the kernel. */

struct delegate_config_t {
  __u32 en_flag;
  __u64 trap_mask;
};

#define KERNEL_BYPASS_HELLO_WORLD _IO(KBE_IOCTL_MAGIC, 0x30)
#define KERNEL_BYPASS_INSTALL_HANDLER_TARGET _IOW(KBE_IOCTL_MAGIC, 0x31, unsigned long)
#define KERNEL_BYPASS_DELEGATE_TRAPS _IOW(KBE_IOCTL_MAGIC, 0x32, struct delegate_config_t*)
#define KERNEL_BYPASS_CSR_STATUS _IO(KBE_IOCTL_MAGIC, 0x33)

int ioctl_install_handler_address(unsigned long target_addr);
int ioctl_delegate_traps(struct delegate_config_t trap_setup);
int ioctl_csr_status(void);

/** Getting KBE'd page faults back into the kernel. */

struct kbe_page_fault_t {
    enum page_fault_kind { CODE, LOAD, STORE } kind;
    /* The virtual address that triggered the page fault.
     *
     * NOTE: This is NOT the address of the instruction that caused the page
     * fault, but the address the instruction was attempting to load from or
     * store to. In a code fault, it is the address of the code page that was
     * attempted to be fetched. */
    __u64 fault_vaddr;
};

#define KERNEL_BYPASS_HANDLE_PAGE_FAULT _IOW(KBE_IOCTL_MAGIC, 0x34, struct kbe_page_fault_t*)

int ioctl_handle_kbe_page_fault(struct kbe_page_fault_t fault);

#endif /* KERNEL_BYPASS_IOCTL_H */
