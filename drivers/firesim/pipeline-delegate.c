#define pr_fmt(fmt) "pipelined-delegate:" KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/moduleparam.h>

#include <asm/io.h>
#include <asm/ptrace.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "pipeline-delegate."
#define MODULE_NAME "pipelined-delegate"

#define DEVICE_NAME "pipelined-delegate"
#define BUF_LEN 100 // Length of buffer in bytes.

#define MAX_MINOR_DEVICES 1

/* The device data is a global struct that keeps track of the cdev's major
 * number. */
static struct pipeline_delegate_char_device_data {
  struct device *device;
  struct cdev cdev;
} pipeline_delegate_dev_data;

static struct class *pipeline_delegate_dev_class;
static int major_device_number;

/** Change the RWX bits of the /dev file created by the device_create call in
 * create_char_devs. */
static int pipeline_delegate_uevent(const struct device *dev, struct kobj_uevent_env *env)
{
  add_uevent_var(env, "DEVMODE=%#o", 0666);
  return 0;
}

/* The function passed to the open field of the file_operations struct should
 * set everything up for the file to be used. This means bringing the seek pointer
 * to a certain file, setting up device minor numbers, allocating memory space
 * for the device file's private information, and so on. */
static int pipelined_delegate_open(struct inode *inode, struct file *filep) {
  pr_info("Opened the pipelined delegation character device file\n");
  return 0;
}

/* The function passed to the release field of the file_operations struct should
 * clean everything up when this instance of the file being opened is closed.
 * This will involve kfree-ing everything that was allocated in the open
 * function. */
static int pipelined_delegate_release(struct inode *inode, struct file *filep) {
  pr_info("Closed the pipelined delegation character device file\n");
  return 0;
}

static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = pipelined_delegate_open,
  .release = pipelined_delegate_release,
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

  pipeline_delegate_dev_class = class_create("pipelined-delegate");
  /* pipeline_delegate_dev_class = class_create(THIS_MODULE, "Pipelined Delegation Char Class"); */
  if (!pipeline_delegate_dev_class) {
    pr_alert("Could not create character class for device\n");
    goto could_not_alloc_chr_region;
  }
  pipeline_delegate_dev_class->dev_uevent = pipeline_delegate_uevent;

  // Initialize cdev with these possible file operations.
  cdev_init(&pipeline_delegate_dev_data.cdev, &fops);
  pipeline_delegate_dev_data.cdev.owner = THIS_MODULE;
  /* Add char device to system. Use MKDEV to create a new dev_t integer
   * with the device's corresponding minor device number. In the case of a
   * single minor device, it is the same as using the dev_t directly. */
  error = cdev_add(&pipeline_delegate_dev_data.cdev,
                   MKDEV(major_device_number, MAX_MINOR_DEVICES - 1), 1);
  if (error) {
    goto could_not_add_cdev;
  }

  /* Create the device and register with sysfs, also creating the entry in
   * /dev mapping to the proper major,minor number. */
  pipeline_delegate_dev_data.device = device_create(pipeline_delegate_dev_class,
                                                    NULL,
                                                    MKDEV(major_device_number, 0),
                                                    NULL, DEVICE_NAME);
  pr_info("Successfully created /dev/" DEVICE_NAME "\n");
  return 0;

could_not_add_cdev:
  cdev_del(&pipeline_delegate_dev_data.cdev);
  class_destroy(pipeline_delegate_dev_class);
could_not_alloc_chr_region:
  unregister_chrdev_region(MKDEV(major_device_number, 0), MAX_MINOR_DEVICES);
  return error;
}

int destroy_char_devs(void)
{
  pr_info("Destroying interactive character devices\n");

  // Destroy the major:minor device
  device_destroy(pipeline_delegate_dev_class, MKDEV(major_device_number, 0));

  pr_debug("Deleting kernel's cdev of device\n");
  cdev_del(&pipeline_delegate_dev_data.cdev);

  pr_debug("Unregistering and Destroying character device class\n");
  class_destroy(pipeline_delegate_dev_class);

  pr_debug("Unregistering and destroying %d character devices with major number %d region\n", MAX_MINOR_DEVICES, major_device_number);
  unregister_chrdev_region(MKDEV(major_device_number, 0), MAX_MINOR_DEVICES);

  return 0;
}

static int __init pipeline_delegate_init(void)
{
  pr_info("Starting Pipelined Delegation module\n");

  int rc = 0;

  rc = create_char_devs();

  struct pt_regs *regs = task_pt_regs(current);
  pr_info("a0: 0x" REG_FMT "\n", regs->a0);

  return 0;
}

static void __exit pipeline_delegate_exit(void)
{
  pr_info("Stopping Pipelined Delegation module\n");
  int rc = 0;
  rc = destroy_char_devs();
}

module_init(pipeline_delegate_init);
module_exit(pipeline_delegate_exit);
MODULE_VERSION("0.0");
MODULE_DESCRIPTION("Work with Pipelined Delegation");
MODULE_LICENSE("UNKNOWN");
