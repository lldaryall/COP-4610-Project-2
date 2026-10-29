#include <linux/init.h>
#include <linux/module.h>

#include "elevator.h"

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Pet elevator scheduler");

static int __init elevator_init(void)
{
	return elevator_thread_init();
}

static void __exit elevator_exit(void)
{
	elevator_thread_exit();
}

module_init(elevator_init);
module_exit(elevator_exit);
