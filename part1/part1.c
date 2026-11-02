/*
 * part1.c - Part 1 program
 *
 * Requirement: make exactly FIVE more system calls than empty.c.
 *
 * How it works:
 *   Both programs are compiled the same way (see Makefile), so the startup
 *   and shutdown syscalls described in empty.c are identical in both
 *   traces. The only difference is the body of main() below.
 *
 *   Each call in main() goes through a thin glibc wrapper that makes
 *   exactly one system call and nothing else:
 *
 *     write()   -> write(2)    sends bytes to a file descriptor
 *     getpid()  -> getpid(2)   gets this process's ID
 *     getppid() -> getppid(2)  gets the parent's ID (the shell or strace)
 *     getuid()  -> getuid(2)   gets the real user ID
 *     getgid()  -> getgid(2)   gets the real group ID
 *
 *   glibc stopped caching getpid() in version 2.25, so every call really
 *   enters the kernel.
 *
 * Why not printf()?
 *   printf() goes through stdio buffering. The first time stdout is used,
 *   glibc calls fstat/newfstatat to decide how to buffer it and may call
 *   brk to allocate the buffer. Those extra syscalls depend on the
 *   environment, which would make the count unpredictable. Calling write()
 *   directly guarantees exactly one syscall.
 *
 * Verification:
 *   `make check` traces both programs and confirms part1.trace has exactly
 *   5 more lines than empty.trace (strace prints one line per syscall).
 */
#include <unistd.h>	/* write, getpid, getppid, getuid, getgid */

int main(void)
{
	/* static const: stored in read-only data, so no allocation syscalls. */
	static const char msg[] = "part1: making five system calls\n";

	/*
	 * Syscall 1: write(2) to stdout (fd 1). sizeof(msg) - 1 leaves out
	 * the trailing '\0'. The return value is checked because glibc marks
	 * write() as warn_unused_result when FORTIFY_SOURCE is enabled.
	 */
	if (write(STDOUT_FILENO, msg, sizeof(msg) - 1) < 0)
		return 1;

	/*
	 * Syscalls 2-5: simple ID queries. The results are ignored; we only
	 * need the kernel calls to show up in the trace. The compiler can't
	 * optimize these away because they are external library functions.
	 */
	getpid();	/* Syscall 2 */
	getppid();	/* Syscall 3 */
	getuid();	/* Syscall 4 */
	getgid();	/* Syscall 5 */

	/* Same exit path as empty.c: one exit_group(0), present in both traces. */
	return 0;
}
