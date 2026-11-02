#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "elevator.h"

extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

static int start_elevator(void)
{
	return elevator_activate();
}

static int issue_request(int start_floor, int destination_floor, int type)
{
	int ret = elevator_queue_request(start_floor, destination_floor, type);

	return ret == -EINVAL ? 1 : ret;
}

static int stop_elevator(void)
{
	return elevator_deactivate();
}

void elevator_syscalls_register(void)
{
	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;
}

void elevator_syscalls_unregister(void)
{
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}
