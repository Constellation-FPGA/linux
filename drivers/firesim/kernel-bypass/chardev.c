#include "config.h"
#include "ioctl.h"
#include "chardev.h"

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>

#define DEVICE_NAME MODULE_NAME

#define MAX_MINOR_DEVICES 1

/* The device data is a global struct that keeps track of the cdev's major
 * number. */
static struct kernel_bypass_char_device_data {
  struct device *device;
  struct cdev cdev;
} kernel_bypass_dev_data;

static struct class *kernel_bypass_dev_class;
static int major_device_number;

/** Change the RWX bits of the /dev file created by the device_create call in
 * create_char_devs. */
static int kernel_bypass_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0;
}

/* The function passed to the open field of the file_operations struct should
 * set everything up for the file to be used. This means bringing the seek pointer
 * to a certain file, setting up device minor numbers, allocating memory space
 * for the device file's private information, and so on. */
static int kernel_bypass_open(struct inode *inode, struct file *filep)
{
  pr_info("Opened the Kernel-Bypass Exception character device file\n");
  return 0;
}

/* The function passed to the release field of the file_operations struct should
 * clean everything up when this instance of the file being opened is closed.
 * This will involve kfree-ing everything that was allocated in the open
 * function. */
static int kernel_bypass_release(struct inode *inode, struct file *filep)
{
  int rc = 0;
  pr_info("Closed the Kernel-Bypass Exception character device file\n");
  pr_info("Resetting Kernel-Bypass Exception CSRs!\n");
  rc = ioctl_install_handler_address(0UL);
  struct delegate_config_t reset = {.en_flag = 0, .trap_mask = -1UL};
  rc = ioctl_delegate_traps(reset);
  pr_info("Closed Kernel-Bypass Exception character device file %s\n",
          rc == 0 ? "succesfully" : "failed");
  return rc;
}

static long kernel_bypass_ioctl(struct file *filep, unsigned int cmd, unsigned long args)
{
  /* struct kernel_bypass_private_data *priv = filep->private_data; */

  /* TODO: Dump all of pt_regs in hex somehow. */
  struct pt_regs *regs = task_pt_regs(current);
  pr_debug("Before ioctl SEPC: " REG_FMT "\n", csr_read(CSR_EPC));
  pr_debug("Before ioctl pt_regs->epc: " REG_FMT "\n", regs->epc);
  pr_debug("pt_regs addr: 0x%016lx\n", (unsigned long)regs);

  long ret = -ENOTTY;
  switch(cmd) {
  case KERNEL_BYPASS_HELLO_WORLD:
    pr_info("HELLO WORLD!\n");
    ret = 0;
    break;
  case KERNEL_BYPASS_INSTALL_HANDLER_TARGET: {
    unsigned long target_addr = args;
    ret = ioctl_install_handler_address(target_addr);
    break;
  }
  case KERNEL_BYPASS_DELEGATE_TRAPS: {
    struct delegate_config_t trap_setup;
    ret = copy_from_user(&trap_setup, (struct delegate_config_t*) args,
                         sizeof(struct delegate_config_t));
    if (ret) {
      pr_alert("Huh? Only copied %ld bytes from user-space... weird\n",
	       (sizeof(struct delegate_config_t) - ret));
      break;
    }

    ret = ioctl_delegate_traps(trap_setup);
    break;
  }
  case KERNEL_BYPASS_HANDLE_PAGE_FAULT: {
      struct kbe_page_fault_t page_fault;
      ret = copy_from_user(&page_fault, (struct kbe_page_fault_t*) args,
			   sizeof(struct kbe_page_fault_t));
      if (ret) {
	  pr_alert("Huh? Only copied %ld bytes from user-space... weird\n",
		   (sizeof(struct kbe_page_fault_t) - ret));
	  break;
      }

      ret = ioctl_handle_kbe_page_fault(page_fault);
      break;
  }
  case KERNEL_BYPASS_IOCTL_TIME: {
      struct kbe_ioctl_time_t times = { 0 };
      ioctl_handle_time(&times);
      ret = copy_to_user((struct kbe_ioctl_time_t*) args, &times,
			 sizeof(struct kbe_ioctl_time_t));
      if (ret) {
	  pr_alert("Only copied %ld bytes of ioctl times to user-space\n",
		   (sizeof (struct kbe_ioctl_time_t) - ret));
	  break;
      }
      break;
  }
  case KERNEL_BYPASS_CSR_STATUS:
    ret = ioctl_csr_status();
    break;
  default:
    pr_alert("Received unsupported ioctl: 0x%x\n", cmd);
    ret = -ENOTTY;
    break;
  }

  pr_debug("After ioctl SEPC: " REG_FMT "\n", csr_read(CSR_EPC));
  pr_debug("After ioctl pt_regs->epc: " REG_FMT "\n", regs->epc);
  pr_debug("Finished ioctl!\n");
  return ret;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = kernel_bypass_open,
  .release = kernel_bypass_release,
  .unlocked_ioctl = kernel_bypass_ioctl,
};

int create_char_devs(void)
{
  int error;
  dev_t char_dev;

  pr_info("Creating interactive character devices\n");

  /* Allocate a major device and minor numbers for this module. */
  error = alloc_chrdev_region(&char_dev, 0, MAX_MINOR_DEVICES, MODULE_NAME);
  if (error) {
    pr_alert("Could not allocate major number for device\n");
    goto could_not_alloc_chr_region;
  }

  major_device_number = MAJOR(char_dev);
  pr_debug("Major Device Number: %d", major_device_number);

  kernel_bypass_dev_class = class_create("kernel-bypass");
  /* kernel_bypass_dev_class = class_create(THIS_MODULE, "Kernel-Bypass Exception Char Class"); */
  if (!kernel_bypass_dev_class) {
    pr_alert("Could not create character class for device\n");
    goto could_not_alloc_chr_region;
  }
  kernel_bypass_dev_class->dev_uevent = kernel_bypass_uevent;

  // Initialize cdev with these possible file operations.
  cdev_init(&kernel_bypass_dev_data.cdev, &fops);
  kernel_bypass_dev_data.cdev.owner = THIS_MODULE;
  /* Add char device to system. Use MKDEV to create a new dev_t integer
   * with the device's corresponding minor device number. In the case of a
   * single minor device, it is the same as using the dev_t directly. */
  error = cdev_add(&kernel_bypass_dev_data.cdev,
                   MKDEV(major_device_number, MAX_MINOR_DEVICES - 1), 1);
  if (error) {
    goto could_not_add_cdev;
  }

  /* Create the device and register with sysfs, also creating the entry in
   * /dev mapping to the proper major,minor number. */
  kernel_bypass_dev_data.device = device_create(kernel_bypass_dev_class,
                                                    NULL,
                                                    MKDEV(major_device_number, 0),
                                                    NULL, DEVICE_NAME);
  pr_info("Successfully created /dev/" DEVICE_NAME "\n");
  return 0;

could_not_add_cdev:
  cdev_del(&kernel_bypass_dev_data.cdev);
  class_destroy(kernel_bypass_dev_class);
could_not_alloc_chr_region:
  unregister_chrdev_region(MKDEV(major_device_number, 0), MAX_MINOR_DEVICES);
  return error;
}

int destroy_char_devs(void)
{
  pr_info("Destroying interactive character devices\n");

  // Destroy the major:minor device
  device_destroy(kernel_bypass_dev_class, MKDEV(major_device_number, 0));

  pr_debug("Deleting kernel's cdev of device\n");
  cdev_del(&kernel_bypass_dev_data.cdev);

  pr_debug("Unregistering and Destroying character device class\n");
  class_destroy(kernel_bypass_dev_class);

  pr_debug("Unregistering and destroying %d character devices with major number %d region\n",
           MAX_MINOR_DEVICES, major_device_number);
  unregister_chrdev_region(MKDEV(major_device_number, 0), MAX_MINOR_DEVICES);

  return 0;
}
