/*
 * syscall_test.c - calls the elevator syscalls directly (Part 3a testing)
 *
 * WHY THIS EXISTS
 *   glibc has no wrapper functions for our custom syscalls (there is no
 *   start_elevator() in libc). Instead we use the generic syscall(2)
 *   function, which takes a syscall number plus arguments and runs the
 *   CPU's syscall instruction with them. This lets us test the kernel
 *   changes from Part 3a/3b without the provided producer/consumer
 *   programs.
 *
 * USAGE
 *   ./syscall_test start                        call start_elevator()
 *   ./syscall_test issue <start> <dest> <type>  call issue_request(...)
 *   ./syscall_test stop                         call stop_elevator()
 *   ./syscall_test smoke                        run a scripted pass/fail test
 *
 * WHAT THE RESULTS MEAN
 *   -1 / ENOSYS  "Function not implemented". Either:
 *                  - you booted the stock kernel (syscall 548+ isn't in
 *                    the table at all), check `uname -r`, or
 *                  - you're on the new kernel but elevator.ko isn't
 *                    loaded (the STUB_* pointers are NULL)
 *   0            success
 *   1            the module rejected the call (already started, invalid
 *                request, already stopping)
 */
#include <errno.h>	/* errno */
#include <stdio.h>	/* printf, fprintf, snprintf */
#include <stdlib.h>	/* atoi */
#include <string.h>	/* strcmp, strerror */
#include <sys/syscall.h>
#include <unistd.h>	/* syscall() */

/*
 * Syscall numbers. These must match the entries added to
 * arch/x86/entry/syscalls/syscall_64.tbl by kernel/apply_syscalls.sh.
 */
#define __NR_START_ELEVATOR 548
#define __NR_ISSUE_REQUEST  549
#define __NR_STOP_ELEVATOR  550

/*
 * report - print one syscall's result and pass the value back
 *
 * syscall() returns -1 and sets errno when the kernel returns a negative
 * error code (e.g. -ENOSYS). In that case we also print the errno text.
 * Otherwise the return value is the module's own result (0 or 1).
 */
static long report(const char *name, long ret)
{
	if (ret < 0)
		printf("%-28s -> %ld (%s)\n", name, ret, strerror(errno));
	else
		printf("%-28s -> %ld\n", name, ret);
	return ret;
}

/* Calls syscall 548 with no arguments. */
static long do_start(void)
{
	return report("start_elevator()", syscall(__NR_START_ELEVATOR));
}

/* Calls syscall 549. The three ints go to the kernel in registers. */
static long do_issue(int s, int d, int t)
{
	char label[64];

	snprintf(label, sizeof(label), "issue_request(%d, %d, %d)", s, d, t);
	return report(label, syscall(__NR_ISSUE_REQUEST, s, d, t));
}

/* Calls syscall 550 with no arguments. */
static long do_stop(void)
{
	return report("stop_elevator()", syscall(__NR_STOP_ELEVATOR));
}

/*
 * smoke - scripted check of every return code
 *
 * Needs elevator.ko loaded with the elevator OFFLINE. Each line compares
 * the actual result with the expected one and ORs any mismatch into
 * `fail`, so the whole sequence runs even if an early step fails.
 */
static int smoke(void)
{
	int fail = 0;

	/* Starting: first call succeeds, second reports "already active". */
	fail |= do_start() != 0;
	fail |= do_start() != 1;

	/* Valid requests: floors 1-5, pet types 0-3. */
	fail |= do_issue(1, 5, 0) != 0;		/* chihuahua, 1 -> 5 */
	fail |= do_issue(5, 1, 3) != 0;		/* dachshund, 5 -> 1 */

	/* Invalid requests: each one should be rejected with 1. */
	fail |= do_issue(0, 3, 1) != 1;		/* start floor below 1 */
	fail |= do_issue(2, 6, 1) != 1;		/* destination above 5 */
	fail |= do_issue(2, 3, 4) != 1;		/* pet type past the last one */
	fail |= do_issue(2, 3, -1) != 1;	/* negative pet type */
	fail |= do_issue(3, 3, 1) != 1;		/* start equals destination */

	/* Stopping: should be accepted. */
	fail |= do_stop() != 0;

	printf("%s\n", fail ? "SMOKE TEST FAILED" : "SMOKE TEST PASSED");
	return fail;
}

/*
 * main - choose a mode from the command line
 *
 * Exit status: 0 on success, 1 on failure, 2 on bad usage, so the program
 * can be used in shell scripts (e.g. `./syscall_test smoke && echo ok`).
 */
int main(int argc, char **argv)
{
	if (argc == 2 && strcmp(argv[1], "start") == 0)
		return do_start() < 0;
	if (argc == 5 && strcmp(argv[1], "issue") == 0)
		return do_issue(atoi(argv[2]), atoi(argv[3]), atoi(argv[4])) < 0;
	if (argc == 2 && strcmp(argv[1], "stop") == 0)
		return do_stop() < 0;
	if (argc == 2 && strcmp(argv[1], "smoke") == 0)
		return smoke();

	/* Anything else: print usage. */
	fprintf(stderr,
		"usage: %s start | stop | smoke | issue <start> <dest> <type>\n",
		argv[0]);
	return 2;
}
