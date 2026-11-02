COP 4610 Project 2: Linux Kernel Programming

This project covers system call tracing, a timer kernel module, and an elevator
kernel module that transports pets between floors.

 ## Team Members 
 - Darya Pylypenko (dp24s)
 - Cole Dunlop (cjd21d)
 - Ermithe Tilusca (et24f)

## Repository Layout

```text
part1/                  
part2/
  timer.c               Timer kernel module source; creates /proc/timer
  Makefile              Builds the module as my_timer.ko
  VERIFY.md             Part 2 build and load/read/unload record
part3/
  Makefile              External-module build file
  src/
    elevator_module.c   Module initialization and cleanup
    elevator_thread.c   Elevator kernel thread and lifecycle
    elevator_sched.c    Movement and pet loading/unloading logic
    elevator.h          Elevator declarations and shared definitions
    pet_queues_mutex.c  Per-floor FIFO queues, passenger list, and mutex
    pet_queues_mutex.h  Shared elevator and pet data definitions

## Part 2: Timer Module

The timer module uses `ktime_get_real_ts64()` and exposes the result through
`/proc/timer`. Its first read reports current time; later reads also report
elapsed time since the prior read.

Build and verify it on the Linux VM with matching kernel headers:

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

The tested VM used kernel `7.0.0-31-generic`. The recorded build and runtime
results are in `part2/VERIFY.md`.

## Part 3: Pet Elevator

The current source includes the elevator thread, scheduler, shared pet queues,
and mutex-protected state. The thread and scheduler use five floor queues and
one passenger list.

**Integration still in progress:** the current repository tree does not yet
include the `start_elevator`, `issue_request`, and `stop_elevator` system-call
handlers or the `/proc/elevator` status interface. The kernel source changes
and full-kernel build are also performed outside this repository.

## Team Responsibilities

| Part | Primary | Support |
| --- | --- | --- |
| 1: System call tracing | Cole | Ermithe: test and verify |
| 2: Timer module | Ermithe | Ermithe: test and verify |
| 3a: System calls | Cole | Darya: test |
| 3b: Kernel compilation | Cole | Ermithe: troubleshoot and verify |
| 3c: Elevator thread | Darya | Cole: test |
| 3d: Linked lists | Ermithe | Darya: integrate and test |
| 3e: Mutexes | Ermithe | Darya: integrate and test |
| 3f: Scheduling | Darya | Cole: test |

Each team member should record their contributions in the development log and
help test the integrated project before the demonstration.

