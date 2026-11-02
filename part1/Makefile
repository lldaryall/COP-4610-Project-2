# Part 1 Makefile: System Call Tracing
#
# Targets:
#   make          build the empty and part1 executables
#   make traces   run both under strace, writing empty.trace and part1.trace
#   make check    make traces, then verify part1 has exactly 5 extra syscalls
#   make clean    remove the executables (the .trace files are deliverables
#                 and are kept)
#
# The traces must be generated on the Linux VM: strace uses the Linux
# ptrace interface and does not exist on macOS.

# Both programs use the same compiler and flags. That matters: different
# flags (e.g. -static) would change the startup syscalls and break the
# "exactly 5 more" comparison.
#   -Wall -Wextra  turn on most warnings
#   -O0            no optimization, so the code runs as written
CC     = gcc
CFLAGS = -Wall -Wextra -O0

# Default target: build both executables.
all: empty part1

# $@ = target name (empty), $< = first prerequisite (empty.c).
empty: empty.c
	$(CC) $(CFLAGS) -o $@ $<

part1: part1.c
	$(CC) $(CFLAGS) -o $@ $<

# strace runs each program and logs every syscall it makes, one per line,
# to the file given with -o. The last line of each file is strace's own
# "+++ exited with 0 +++" marker, which appears once in both.
traces: empty part1
	strace -o empty.trace ./empty
	strace -o part1.trace ./part1

# Count the lines in each trace and compare. Since strace writes one line
# per syscall and both files share the same exit marker, the line-count
# difference equals the syscall-count difference.
# In recipes, $$ passes a literal $ through to the shell.
check: traces
	@e=$$(wc -l < empty.trace); p=$$(wc -l < part1.trace); \
	echo "empty: $$e lines, part1: $$p lines, difference: $$((p - e))"; \
	test $$((p - e)) -eq 5 && echo "PASS" || (echo "FAIL: expected 5"; exit 1)

clean:
	rm -f empty part1

# These targets are commands, not files, so make should always run them.
.PHONY: all traces check clean
