#ifndef PIPELINED_DELEGATE_IOCTL_H
#define PIPELINED_DELEGATE_IOCTL_H

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

#define PIPELINED_DELEGATE_HELLO_WORLD _IO(IOCTL_MAGIC, 0x30)
#define PIPELINED_DELEGATE_INSTALL_HANDLER_TARGET _IOR(IOCTL_MAGIC, 0x31, unsigned long)
#define PIPELINED_DELEGATE_DELEGATE_TRAPS _IOR(IOCTL_MAGIC, 0x32, struct delegate_config_t*)

int ioctl_install_handler_address(unsigned long target_addr);
int ioctl_delegate_traps(struct delegate_config_t trap_setup);

#endif /* PIPELINED_DELEGATE_IOCTL_H */
