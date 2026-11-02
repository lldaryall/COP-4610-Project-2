/*
 * syscalls.c - kernel-side elevator system calls (Part 3a)
 *
 * WHERE THIS FILE LIVES
 *   This file is built into the kernel image itself, not into the module.
 *   kernel/apply_syscalls.sh copies it to <kernel-src>/syscalls/syscalls.c
 *   and creates a Makefile beside it containing "obj-y := syscalls.o"
 *   (obj-y means "build into the kernel"; obj-m would mean "build as a
 *   module").
 *
 * THE PROBLEM IT SOLVES
 *   A system call has to exist when the kernel boots, because the syscall
 *   table is fixed at compile time. But the elevator logic lives in a
 *   loadable module (elevator.ko) that we insmod and rmmod over and over
 *   during development. Rebuilding the whole kernel for every elevator
 *   change would take far too long.
 *
 * THE SOLUTION: FUNCTION-POINTER STUBS
 *   1. The kernel defines one function pointer per syscall (STUB_*),
 *      starting out as NULL.
 *   2. EXPORT_SYMBOL makes those pointers visible to modules, so
 *      elevator.ko can declare them "extern" and assign to them.
 *   3. When elevator.ko loads, it points each STUB_* at its own handler.
 *      When it unloads, it sets them back to NULL.
 *   4. Each syscall below checks its pointer: if a module has set it,
 *      the call is forwarded to the module; if not, it returns -ENOSYS
 *      ("function not implemented").
 *
 * HOW A CALL FLOWS
 *   user program: syscall(549, 1, 5, 0)
 *     -> CPU "syscall" instruction enters the kernel with rax = 549
 *     -> x86 entry code looks up entry 549 in sys_call_table
 *        (generated from syscall_64.tbl at build time)
 *     -> __x64_sys_issue_request(regs)  (wrapper made by SYSCALL_DEFINE3)
 *     -> our body below, with start_floor=1, destination_floor=5, type=0
 *     -> STUB_issue_request(...)  i.e. issue_request() in elevator.ko
 *     -> return value goes back to user space in rax. A negative value
 *        like -ENOSYS makes glibc's syscall() return -1 and set errno.
 */
#include <linux/errno.h>		/* ENOSYS */
#include <linux/linkage.h>	/* asmlinkage */
#include <linux/kernel.h>	/* core kernel definitions */
#include <linux/module.h>	/* EXPORT_SYMBOL */
#include <linux/syscalls.h>	/* SYSCALL_DEFINEn macros, our prototypes */

/*
 * One pointer per syscall, with the same signature as the module's
 * handler. NULL means "no elevator module loaded".
 */
int (*STUB_start_elevator)(void) = NULL;
int (*STUB_issue_request)(int, int, int) = NULL;
int (*STUB_stop_elevator)(void) = NULL;

/*
 * Add the pointers to the kernel's exported symbol table. Without this,
 * insmod elevator.ko fails with "Unknown symbol STUB_start_elevator".
 */
EXPORT_SYMBOL(STUB_start_elevator);
EXPORT_SYMBOL(STUB_issue_request);
EXPORT_SYMBOL(STUB_stop_elevator);

/*
 * SYSCALL_DEFINEn(name, type1, arg1, type2, arg2, ...)
 *   n is the number of arguments. The macro generates:
 *     - sys_<name>: the function that syscall_64.tbl refers to
 *     - __x64_sys_<name>: an x86-64 wrapper that pulls the arguments out
 *       of the saved user registers (struct pt_regs) and calls our body
 *     - sign-extension and type-checking glue for each argument
 *   Types and names are given as separate macro arguments ("int,
 *   start_floor"), which is why it looks different from a normal function.
 *
 * READ_ONCE(ptr)
 *   Copies the pointer into a local variable exactly once. Without it, the
 *   compiler could read the global twice (once for the NULL check, once
 *   for the call). If rmmod set it to NULL between those reads, the kernel
 *   would call address 0 and crash. With the local copy, the check and
 *   the call always use the same value.
 *   (This narrows the window but doesn't fully protect against unloading
 *   the module during a call. Don't rmmod while a test program is
 *   running.)
 */

/* Syscall 548: int start_elevator(void) */
SYSCALL_DEFINE0(start_elevator)
{
	int (*fn)(void) = READ_ONCE(STUB_start_elevator);

	if (fn)
		return fn();	/* module loaded: forward to its handler */
	return -ENOSYS;		/* module not loaded */
}

/* Syscall 549: int issue_request(int start_floor, int destination_floor, int type) */
SYSCALL_DEFINE3(issue_request, int, start_floor, int, destination_floor, int, type)
{
	int (*fn)(int, int, int) = READ_ONCE(STUB_issue_request);

	/*
	 * Arguments are passed through as-is. Validation (floor range, pet
	 * type) happens in the module so it can change without a kernel
	 * rebuild.
	 */
	if (fn)
		return fn(start_floor, destination_floor, type);
	return -ENOSYS;
}

/* Syscall 550: int stop_elevator(void) */
SYSCALL_DEFINE0(stop_elevator)
{
	int (*fn)(void) = READ_ONCE(STUB_stop_elevator);

	if (fn)
		return fn();
	return -ENOSYS;
}
