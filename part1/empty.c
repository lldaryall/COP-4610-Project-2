/*
 * empty.c - Part 1 baseline program
 *
 * This program does nothing on purpose. Tracing it with strace shows the
 * system calls every dynamically linked C program makes before main()
 * runs and after it returns:
 *
 *   - execve        the shell asks the kernel to load this binary
 *   - brk, mmap,    the dynamic loader (ld-linux) sets up memory, finds
 *     openat, read, libc.so, maps it into the process, and applies memory
 *     mprotect, ...  protections
 *   - arch_prctl,   libc sets up thread-local storage, the robust futex
 *     set_tid_address, list, and other per-thread state
 *     set_robust_list, rseq, prlimit64, getrandom, ...
 *   - exit_group    returning from main() makes libc call exit(), which
 *                   ends the process
 *
 * empty.trace records that baseline. part1.trace should contain the same
 * calls plus exactly five more.
 */
int main(void)
{
	/* Returning 0 from main() leads to the single exit_group(0) syscall. */
	return 0;
}
