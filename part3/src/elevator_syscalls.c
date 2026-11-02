/*
 * elevator_syscalls.c - module-side syscall handlers (Part 3a)
 *
 * This is the module half of the syscall plumbing. The kernel half,
 * part3/syscalls.c, defines three exported function pointers (STUB_*).
 * This file:
 *
 *   1. Defines the real handlers: start_elevator(), issue_request(),
 *      stop_elevator().
 *   2. Forwards each call to the elevator core:
 *
 *        syscall 548  start_elevator()  ->  elevator_activate()
 *        syscall 549  issue_request()   ->  elevator_queue_request()
 *        syscall 550  stop_elevator()   ->  elevator_deactivate()
 *
 *   3. Converts the core's results into the spec's return codes.
 *   4. Gives the module init/exit code functions that set the STUB_*
 *      pointers to these handlers and clear them again.
 *
 * All elevator state and locking belongs to the core. These handlers
 * never touch elevator_state directly, so they don't take the mutex.
 */
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include "elevator.h"

/*
 * The pointers are defined in syscalls.c, which is compiled into the
 * kernel. "extern" says they exist somewhere else. When insmod loads this
 * module, the kernel's module loader resolves them against the exported
 * symbol table (that is what EXPORT_SYMBOL in syscalls.c was for).
 */
extern int (*STUB_start_elevator)(void);
extern int (*STUB_issue_request)(int, int, int);
extern int (*STUB_stop_elevator)(void);

/*
 * The handlers are static: only reachable through the STUB_* pointers,
 * never linked by name from other files.
 */

/*
 * start_elevator - handler for syscall 548
 *
 * elevator_activate() (elevator_thread.c) moves an OFFLINE elevator to
 * IDLE on floor 1. The kthread is already running from module init, so
 * activating it allocates nothing.
 *
 * Returns 0 if started, 1 if the elevator was already active.
 */
static int start_elevator(void)
{
	return elevator_activate();
}

/*
 * issue_request - handler for syscall 549
 * @start_floor:       floor where the pet is waiting (1-5)
 * @destination_floor: floor where the pet wants to go (1-5)
 * @type:              pet type, see enum pet_type (0-3)
 *
 * The values come straight from user space and can be any integers.
 * elevator_queue_request() (pet_queues_mutex.c) checks them before using
 * them: a bad floor would index past the floors array, and a bad type
 * would read past the weight table.
 *
 * It reports a bad request with -EINVAL, but the spec says issue_request()
 * must return 1 for one, so that error is converted here. Anything else
 * (0 on success, -ENOMEM if kmalloc failed) passes through unchanged.
 */
static int issue_request(int start_floor, int destination_floor, int type)
{
	int ret = elevator_queue_request(start_floor, destination_floor, type);

	return ret == -EINVAL ? 1 : ret;
}

/*
 * stop_elevator - handler for syscall 550
 *
 * elevator_deactivate() (elevator_thread.c) only sets a flag. The kthread
 * sees it, stops boarding pets, unloads everyone on board, and then goes
 * OFFLINE. So this returns right away, before the elevator has stopped.
 *
 * Returns 0 if shutdown started, 1 if the elevator is already deactivating
 * or is already OFFLINE.
 */
static int stop_elevator(void)
{
	return elevator_deactivate();
}

/*
 * elevator_syscalls_register - connect the syscalls to this module
 *
 * Called at the END of module init, after all elevator state is ready.
 * Once these assignments run, any process calling syscalls 548-550 lands
 * in the handlers above instead of getting -ENOSYS.
 */
void elevator_syscalls_register(void)
{
	STUB_start_elevator = start_elevator;
	STUB_issue_request = issue_request;
	STUB_stop_elevator = stop_elevator;
}

/*
 * elevator_syscalls_unregister - disconnect the syscalls from this module
 *
 * Called at the START of module exit, before stopping the thread or
 * freeing anything. Once the module's code is unloaded, these pointers
 * would point at freed memory, so they must be NULL by then. After this,
 * the syscalls go back to returning -ENOSYS.
 */
void elevator_syscalls_unregister(void)
{
	STUB_start_elevator = NULL;
	STUB_issue_request = NULL;
	STUB_stop_elevator = NULL;
}
