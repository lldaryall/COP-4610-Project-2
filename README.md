# COP-4610-Project-2: Kernel Module Programming

Project document: https://docs.google.com/document/d/1l4xKTj2uN0BWXK31x9l81HyFRAqtyteGZR2jgpl8Jis/edit?usp=sharing

## Team Members
- Darya Pylypenko (dp24s)
- Cole Dunlop (cjd21d)
- Ermithe Tilusca (et24f)

## Division of Labor

| Part | Primary | Assisting |
|------|---------|-----------|
| 1 – System Call Tracing | Cole | Ermithe |
| 2 – Timer Kernel Module | Ermithe | Darya |
| 3a – Adding System Calls | Cole | Darya |
| 3b – Kernel Compilation | Cole | Ermithe |
| 3c – Threads | Darya | Cole |
| 3d – Linked Lists | Ermithe | Darya |
| 3e – Mutexes | Ermithe | Darya |
| 3f – Scheduling Algorithm | Darya | Cole |

All members share integration, debugging, testing, the README, and demo prep.

## File Listing
```
.
├── Makefile                     builds all three parts
├── README.md
├── timer.c                      TODO: move to part2/src/ and add part2/Makefile
├── part1/
│   ├── empty.c
│   ├── empty.trace              generated on the VM with `make traces`
│   ├── part1.c
│   ├── part1.trace              generated on the VM with `make traces`
│   └── Makefile
└── part3/
    ├── Makefile                 builds elevator.ko
    ├── syscalls.c               kernel-side syscalls 548-550
    ├── src/
    │   ├── elevator.h
    │   ├── elevator_module.c    module init and exit
    │   ├── elevator_proc.c      /proc/elevator
    │   ├── elevator_sched.c     scheduling algorithm
    │   ├── elevator_syscalls.c  module-side syscall handlers
    │   ├── elevator_thread.c    kthread, activate and deactivate
    │   ├── pet_queues_mutex.c   pet lists and shared state
    │   └── pet_queues_mutex.h
    ├── kernel/
    │   ├── apply_syscalls.sh    adds the syscalls to a kernel source tree
    │   └── build_kernel.sh      configures, compiles, and installs the kernel
    └── tests/
        ├── syscall_test.c       calls syscalls 548-550 directly
        └── Makefile
```

## How to Compile and Run

### Prerequisites (Ubuntu VM)
```
sudo apt install build-essential libncurses-dev bison flex libssl-dev libelf-dev dwarves bc strace
```

### Part 1: System Call Tracing
```
cd part1
make check
```
This builds both programs, writes `empty.trace` and `part1.trace`, and verifies
that `part1` makes exactly 5 more system calls than `empty`.

`part1.c` adds `write`, `getpid`, `getppid`, `getuid`, and `getgid`. It skips stdio
because `printf` makes extra buffer-setup system calls.

### Part 2: Timer Kernel Module
TODO

### Part 3: Install the System Calls and Kernel
Run once on the VM, using the Linux 6.16.x source in `/usr/src`:
```
cd part3/kernel
sudo ./apply_syscalls.sh /usr/src/linux-<version>
./build_kernel.sh /usr/src/linux-<version>
sudo reboot
```
After rebooting, confirm the new kernel is running:
```
uname -r
```

### Part 3: Build and Run the Elevator
```
cd part3
make
sudo insmod elevator.ko
```
Test the system calls (run with the elevator OFFLINE):
```
cd tests
make
./syscall_test smoke
```
Watch the elevator, then unload it:
```
watch -n 1 cat /proc/elevator
sudo rmmod elevator
```

## System Calls

| # | System call | Returns |
|---|-------------|---------|
| 548 | `int start_elevator(void)` | 0 started, 1 already active |
| 549 | `int issue_request(int start_floor, int destination_floor, int type)` | 0 valid, 1 invalid, `-ENOMEM` if allocation fails |
| 550 | `int stop_elevator(void)` | 0 stopping, 1 already deactivating or OFFLINE |

`syscalls.c` is built into the kernel and forwards each call through an exported
`STUB_*` function pointer, returning `-ENOSYS` if the module isn't loaded.
`src/elevator_syscalls.c` sets those pointers when the module loads and clears them
when it unloads. Each handler calls `elevator_activate()`, `elevator_queue_request()`,
or `elevator_deactivate()`.

## Development Log
TODO: each member adds dated entries.

### Darya Pylypenko

### Cole Dunlop

### Ermithe Tilusca

## Group Meetings
TODO: date, attendees, and what was covered.
