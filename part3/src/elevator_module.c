#include <linux/init.h>
#include <linux/module.h>

#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pet elevator scheduler");

static int __init elevator_init(void)
{
	int err;

	elevator_shared_init();

	err = elevator_proc_init();
	if (err)
		return err;

	err = elevator_thread_init();
	if (err)
		elevator_proc_exit();
	return err;
}

static void __exit elevator_exit(void)
{
	elevator_proc_exit();
	elevator_thread_exit();
	elevator_shared_cleanup();
}

module_init(elevator_init);
module_exit(elevator_exit);
