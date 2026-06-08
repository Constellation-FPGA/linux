#ifndef KERNEL_BYPASS_IOCTL_H
#define KERNEL_BYPASS_IOCTL_H

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
#define IOCTL_MAGIC 'F'

struct delegate_config_t {
  unsigned int  en_flag;
  unsigned long trap_mask;
};

#define KERNEL_BYPASS_HELLO_WORLD _IO(IOCTL_MAGIC, 0x30)
#define KERNEL_BYPASS_INSTALL_HANDLER_TARGET _IOR(IOCTL_MAGIC, 0x31, unsigned long)
#define KERNEL_BYPASS_DELEGATE_TRAPS _IOR(IOCTL_MAGIC, 0x32, struct delegate_config_t*)
#define KERNEL_BYPASS_CSR_STATUS _IO(IOCTL_MAGIC, 0x33)

int ioctl_install_handler_address(unsigned long target_addr);
int ioctl_delegate_traps(struct delegate_config_t trap_setup);
int ioctl_csr_status(void);

#endif /* KERNEL_BYPASS_IOCTL_H */
