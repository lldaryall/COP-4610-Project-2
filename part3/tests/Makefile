# Builds the user-space syscall test program.
#
# This is an ordinary user program, so it uses plain gcc, not kbuild.
# Build and run it on the VM: it calls Linux-only syscall numbers.
#
#   make                 build ./syscall_test
#   ./syscall_test smoke run the scripted test (needs elevator.ko loaded)
#   make clean           remove the binary

CC     = gcc
CFLAGS = -Wall -Wextra

# $@ = syscall_test, $< = syscall_test.c
syscall_test: syscall_test.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f syscall_test

.PHONY: clean
