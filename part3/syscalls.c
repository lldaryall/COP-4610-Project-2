#include <linux/errno.h>
#include <linux/linkage.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>

int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;

EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

SYSCALL_DEFINE0(start_elevator)
{
	int (*fn)(void) = READ_ONCE(STUB_start_elevator);

	if (fn)
		return fn();
	return -ENOSYS;
}

SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type)
{
	int (*fn)(int, int, int) = READ_ONCE(STUB_issue_request);

	if (fn)
		return fn(start_floor, destination_floor, type);
	return -ENOSYS;
}

SYSCALL_DEFINE0(stop_elevator)
{
	int (*fn)(void) = READ_ONCE(STUB_stop_elevator);

	if (fn)
		return fn();
	return -ENOSYS;
}
