COP 4610 Project 2: Linux Kernel Programming

This team project explores Linux system calls, kernel modules, kernel threads,
linked lists, mutexes, and an elevator scheduler that transports pets between
five floors.

## Team members
- Darya Pylypenko(dp24s)
- Cole Dunlop(cjd21d)
- Ermithe Tilusca(et24f)
## Project Parts

| Part | Work |
| --- | --- |
| 1 | Compare system calls made by a baseline program and a test program. |
| 2 | Build a timer kernel module that reports timestamps through `/proc/timer`. |
| 3 | Add elevator system calls and a kernel module with pet queues, synchronization, and scheduling. |

## Repository Layout

```text
part1/
  empty.c               Baseline program for system-call tracing
  part1.c               Program being compared with the baseline
  empty.trace           System calls recorded for empty.c
  part1.trace           System calls recorded for part1.c
part2/
  timer.c               Timer module implementation
  Makefile              Builds the module as my_timer.ko
  timer_kernel_verify.md Part 2 build and runtime verification record
part3/
  Makefile              External-module build instructions
  src/
    elevator_module.c   Module initialization and cleanup
    elevator_thread.c   Elevator kernel thread and lifecycle
    elevator_sched.c    Elevator movement and passenger scheduling
    elevator.h          Shared elevator declarations and definitions
    pet_queues_mutex.c  Pet queues, passenger list, and mutex operations
    pet_queues_mutex.h  Pet and queue data structures
```

Update this tree if files are renamed or additional project files are added.

## Part 1: System Call Tracing

Part 1 compares the system calls made by `empty.c` with those made by
`part1.c`. The resulting traces are saved as `empty.trace` and `part1.trace`.
The team checks that `part1` makes exactly five more system calls than the
baseline program. Cole is responsible for the programs and trace files; Ermithe
assists with testing and verification.

## Part 2: Timer Kernel Module

The module calls `ktime_get_real_ts64()` to get real time in seconds and
nanoseconds. It creates `/proc/timer` when loaded. Reading that file prints the
current timestamp; after the first read, it also prints the time elapsed since
the previous read. Unloading the module removes the proc entry.

Build and exercise the module on the Linux VM with headers for the running
kernel:

```bash
cd part2
make
sudo insmod my_timer.ko
lsmod | grep my_timer
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer
test ! -e /proc/timer && echo "PASS: /proc/timer removed"
```

I built and tested the module on kernel `7.0.0-31-generic`. I verified that it
loads, appears in `lsmod`, reports current and elapsed time through
`/proc/timer`, and removes `/proc/timer` when unloaded. The build emitted a
compiler-version warning, but completed successfully. The verification record
is in `part2/timer_kernel_verify.md`.

## Part 3: Pet Elevator

The Part 3 source is organized around a kernel thread, elevator scheduling,
per-floor FIFO queues, a passenger list, and mutex-protected shared state.
System-call integration, the `/proc/elevator` status interface, and a complete
kernel build should be marked complete only after they have been added to the
repository and verified together.

## Team Responsibilities

| Part | Primary | Assistance |
| --- | --- | --- |
| 1: System call tracing | Cole | Ermithe: test and verify |
| 2: Timer module | Ermithe | Darya: test and verify |
| 3a: System calls | Cole | Darya: test |
| 3b: Kernel compilation | Cole | Ermithe: troubleshoot and verify |
| 3c: Elevator thread | Darya | Cole: test |
| 3d: Linked lists | Ermithe | Darya: integrate and test |
| 3e: Mutexes | Ermithe | Darya: integrate and test |
| 3f: Scheduling | Darya | Cole: test |

Each team member should record their contributions in the development log.
Before the demonstration, build and test the integrated project on the target
kernel and document any remaining limitations.
