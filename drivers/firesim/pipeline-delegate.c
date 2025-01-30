#define pr_fmt(fmt) "pipelined-delegate:" KBUILD_MODNAME ": " fmt

#include <linux/cpu.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>

#include <asm/ptrace.h>

#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "pipeline-delegate."

static int __init pipeline_delegate_init(void)
{
  pr_info("Starting Pipelined Delegation module\n");
  /* struct task_struct *task; */

  struct pt_regs *regs = task_pt_regs(current);
  pr_info("a0: 0x" REG_FMT "\n", regs->a0);

  return 0;
}

static void __exit pipeline_delegate_exit(void)
{
  pr_info("Stopping Pipelined Delegation module\n");
}

module_init(pipeline_delegate_init);
module_exit(pipeline_delegate_exit);
MODULE_VERSION("0.0");
MODULE_DESCRIPTION("Work with Pipelined Delegation");
MODULE_LICENSE("UNKNOWN");
