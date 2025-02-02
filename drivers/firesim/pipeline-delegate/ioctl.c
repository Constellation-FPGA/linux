#include "config.h"
#include "ioctl.h"

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
