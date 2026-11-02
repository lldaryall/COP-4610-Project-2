# COP-4610-Project-2: Kernel Module Programming

This project covers system call tracing, a timer kernel module, and an elevator
kernel module that transports pets between floors.

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
├── part1/
│   ├── empty.c
│   ├── empty.trace              generated on the VM with `make traces`
│   ├── part1.c
│   ├── part1.trace              generated on the VM with `make traces`
│   └── Makefile
├── part2/
│   ├── Makefile                 builds my_timer.ko
│   ├── timer_kernel_verify.md   build and load/read/unload record
│   └── src/
│       └── my_timer.c           /proc/timer
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
The timer module uses `ktime_get_real_ts64()` and exposes the result through
`/proc/timer`. Its first read reports current time; later reads also report
elapsed time since the prior read.
```
cd part2
make
sudo insmod my_timer.ko
lsmod | grep my_timer
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer
```
The recorded build and runtime results are in `part2/timer_kernel_verify.md`.

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

## Elevator Design

**Thread.** One kthread named `elevator` starts at module load and runs until
unload. Each loop locks the shared mutex, picks the next action, unlocks, sleeps
for that action's delay, then locks again to apply it. The lock is never held
while sleeping, so syscalls and `/proc/elevator` reads don't block on a move.

**States.** OFFLINE until `start_elevator()`, then IDLE when there is no work,
LOADING while pets get on or off, and UP/DOWN while moving. `stop_elevator()`
stops new boarding, delivers the pets already on board, and then goes OFFLINE.
Restarting continues from the floor the elevator stopped on.

**Scheduling.** The elevator keeps going in its current direction while there
is a waiting pet or an onboard pet's destination ahead, and reverses when there
isn't (a LOOK-style sweep). At each floor it stops if a pet gets off there or
if the first pet waiting fits. It unloads first, then boards in FIFO order until
the next pet would exceed 5 pets or 50 lbs. Moving one floor takes 2 s and
loading takes 1 s.

**Pets.** Weights come from the type: Chihuahua 3 lbs, Pug 14 lbs,
Pughuahua 10 lbs, Dachshund 16 lbs. Each request is a `kmalloc`'d node on its
floor's waiting list, moved onto the passenger list when it boards and freed
when it is delivered. Anything still queued is freed on unload.

**/proc/elevator.** Prints the state, current floor, load weight, and onboard
pets, then each floor's waiting pets with `[*]` marking the current floor, then
the onboard, waiting, and serviced counts. It holds the mutex while reading so
the snapshot is consistent.

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

| Date | Work Completed / Notes |
|------|------------------------|
| 2026-10-29 | Elevator kthread: runs from module load to unload, with `elevator_activate()` / `elevator_deactivate()` for start/stop; goes OFFLINE only once empty |
| 2026-10-30 | Scheduling algorithm: one floor per 2 s, 1 s load/unload, FIFO boarding within 5 pets / 50 lbs, reverses when nothing is left ahead |
| 2026-10-30 | Integrated Ermithe's pet lists and mutex into the thread and scheduler; moved them into `part3/src/` |
| 2026-10-30 | `/proc/elevator` in the spec's format; root Makefile |
| 2026-11-02 | Merged Cole's syscall branch with main and combined the READMEs |
| 2026-11-02 | Restarting the elevator now continues from the floor it stopped on instead of resetting to floor 1 |
| 2026-11-02 | Generated `empty.trace` / `part1.trace` (difference of 5); made `KDIR` overridable in the part3 Makefile; added `.gitattributes` so scripts keep Linux line endings |
| 2026-11-02 | Built Linux 6.16.12 with our syscalls and tested in a VM: timer module, syscall smoke test, producer/consumer run, stress run, and unload while running; no warnings or kernel errors |
| 2026-11-02 | wrote the Elevator Design section of the README |

### Cole Dunlop

### Ermithe Tilusca

| Date | Work Completed / Notes |
| --- | --- |
| 2026-10-29 | Added the timer module proc implementation. Used `ktime_get_real_ts64()` to report current time and elapsed time through `/proc/timer`. |
| 2026-10-30 | Implemented the pet linked lists and mutex-protected shared state for the floor queues and elevator passenger list. |
| 2026-11-01 | Built and verified Part 2: loaded the timer module, checked it with `lsmod`, read current and elapsed time from `/proc/timer`, then unloaded it and confirmed the proc entry was removed. Commit: `part 2 verify`. |
| 2026-11-02 | Ran the Part 1 trace comparison on the VM used for Part 2 and verified that `part1` produced exactly five more system calls than `empty`. |
| 2026-11-02 | Merged Cole's syscall branch (PR #10) and updated the README with project details, repository structure, and team information. 
## Group Meetings
TODO: date, attendees, and what was covered.
