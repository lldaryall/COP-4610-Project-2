#ifndef ELEVATOR_H
#define ELEVATOR_H

#include <linux/mutex.h>
#include <linux/sched.h>
#include <linux/types.h>

#define MIN_FLOOR 1
#define MAX_FLOOR 5
#define MAX_PETS 5
#define MAX_WEIGHT 50

#define MOVE_DELAY_MS 2000
#define LOAD_DELAY_MS 1000

#define IDLE_POLL_MS 100

enum elevator_state {
	OFFLINE,
	IDLE,
	LOADING,
	UP,
	DOWN,
};

struct elevator {
	struct mutex lock;
	struct task_struct *kthread;

	enum elevator_state state;
	int current_floor;
	int current_load;
	int num_on_board;
	int num_serviced;

	bool deactivating;

	/* TODO: pet list */
};

extern struct elevator elevator;

int elevator_thread_init(void);
void elevator_thread_exit(void);
int elevator_activate(void);
int elevator_deactivate(void);
const char *elevator_state_name(enum elevator_state state);

int elevator_next_action(void);
void elevator_complete_action(void);

#endif
