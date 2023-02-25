#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>

static int code_init(void) {
	printk(KERN_ALERT "Hello World from Christian 10445108\n");
	return 0;
}

static void code_exit(void) {
	int pid = task_pid_nr(current);
	printk(KERN_ALERT "%s PID = %d\n", current->comm, pid);
}

module_init(code_init);
module_exit(code_exit);
MODULE_LICENSE("Dual BSD/GPL");

