#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/sched.h>
#include <linux/types.h>

#include "pet_queues_mutex.h"

#define MOVE_DELAY_MS 2000
#define LOAD_DELAY_MS 1000
#define IDLE_POLL_MS 100

struct elevator {
	struct task_struct *kthread;
	bool deactivating;
	enum elevator_status direction;
	unsigned int num_serviced;
};

extern struct elevator elevator;

int elevator_thread_init(void);
void elevator_thread_exit(void);
int elevator_activate(void);
int elevator_deactivate(void);
const char *elevator_state_name(enum elevator_status state);

int elevator_proc_init(void);
void elevator_proc_exit(void);

int elevator_next_action(void);
void elevator_complete_action(void);

/*
 * Connect / disconnect syscalls 548-550 (elevator_syscalls.c). Required order:
 *   module init: register LAST, so no syscall reaches half-built state
 *   module exit: unregister FIRST, so no syscall arrives during teardown
 */
void elevator_syscalls_register(void);
void elevator_syscalls_unregister(void);

#endif
