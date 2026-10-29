#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kthread.h>

#include "elevator.h"

struct elevator elevator;

const char *elevator_state_name(enum elevator_state state)
{
	switch (state) {
	case OFFLINE:
		return "OFFLINE";
	case IDLE:
		return "IDLE";
	case LOADING:
		return "LOADING";
	case UP:
		return "UP";
	case DOWN:
		return "DOWN";
	}
	return "UNKNOWN";
}

static int elevator_run_once(void)
{
	int delay_ms;

	if (elevator.state == OFFLINE)
		return IDLE_POLL_MS;

	if (elevator.deactivating && elevator.num_on_board == 0) {
		elevator.state = OFFLINE;
		elevator.deactivating = false;
		return IDLE_POLL_MS;
	}

	delay_ms = elevator_next_action();
	if (delay_ms == 0) {
		elevator.state = IDLE;
		return IDLE_POLL_MS;
	}
	return delay_ms;
}

static int elevator_thread_fn(void *data)
{
	int delay_ms;
	bool acting;

	while (!kthread_should_stop()) {
		mutex_lock(&elevator.lock);
		delay_ms = elevator_run_once();
		acting = elevator.state == LOADING || elevator.state == UP ||
			 elevator.state == DOWN;
		mutex_unlock(&elevator.lock);

		msleep(delay_ms);

		if (acting) {
			mutex_lock(&elevator.lock);
			elevator_complete_action();
			mutex_unlock(&elevator.lock);
		}
	}
	return 0;
}

int elevator_thread_init(void)
{
	mutex_init(&elevator.lock);
	elevator.state = OFFLINE;
	elevator.current_floor = MIN_FLOOR;
	elevator.current_load = 0;
	elevator.num_on_board = 0;
	elevator.num_serviced = 0;
	elevator.deactivating = false;

	elevator.kthread = kthread_run(elevator_thread_fn, NULL, "elevator");
	if (IS_ERR(elevator.kthread)) {
		int err = PTR_ERR(elevator.kthread);

		elevator.kthread = NULL;
		return err;
	}
	return 0;
}

void elevator_thread_exit(void)
{
	if (elevator.kthread) {
		kthread_stop(elevator.kthread);
		elevator.kthread = NULL;
	}
	mutex_lock(&elevator.lock);
	elevator.state = OFFLINE;
	mutex_unlock(&elevator.lock);
}

int elevator_activate(void)
{
	int ret = 0;

	mutex_lock(&elevator.lock);
	if (elevator.state != OFFLINE) {
		ret = 1;
	} else {
		elevator.state = IDLE;
		elevator.current_floor = MIN_FLOOR;
		elevator.current_load = 0;
		elevator.num_on_board = 0;
		elevator.deactivating = false;
	}
	mutex_unlock(&elevator.lock);
	return ret;
}

int elevator_deactivate(void)
{
	int ret = 0;

	mutex_lock(&elevator.lock);
	if (elevator.state == OFFLINE || elevator.deactivating)
		ret = 1;
	else
		elevator.deactivating = true;
	mutex_unlock(&elevator.lock);
	return ret;
}
