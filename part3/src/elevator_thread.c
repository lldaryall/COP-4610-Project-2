#include <linux/delay.h>
#include <linux/err.h>
#include <linux/kthread.h>

#include "elevator.h"

struct elevator elevator;

const char *elevator_state_name(enum elevator_status state)
{
	switch (state) {
	case ELEVATOR_OFFLINE:
		return "OFFLINE";
	case ELEVATOR_IDLE:
		return "IDLE";
	case ELEVATOR_LOADING:
		return "LOADING";
	case ELEVATOR_UP:
		return "UP";
	case ELEVATOR_DOWN:
		return "DOWN";
	}
	return "UNKNOWN";
}

static int elevator_run_once(void)
{
	int delay_ms;

	if (elevator_state.status == ELEVATOR_OFFLINE)
		return IDLE_POLL_MS;

	if (elevator.deactivating && elevator_state.passenger_count == 0) {
		elevator_state.status = ELEVATOR_OFFLINE;
		elevator.deactivating = false;
		return IDLE_POLL_MS;
	}

	delay_ms = elevator_next_action();
	if (delay_ms == 0) {
		elevator_state.status = ELEVATOR_IDLE;
		return IDLE_POLL_MS;
	}
	return delay_ms;
}

static int elevator_thread_fn(void *data)
{
	int delay_ms;
	bool acting;

	while (!kthread_should_stop()) {
		mutex_lock(&elevator_state.lock);
		delay_ms = elevator_run_once();
		acting = elevator_state.status == ELEVATOR_LOADING ||
			 elevator_state.status == ELEVATOR_UP ||
			 elevator_state.status == ELEVATOR_DOWN;
		mutex_unlock(&elevator_state.lock);

		msleep(delay_ms);

		if (acting) {
			mutex_lock(&elevator_state.lock);
			elevator_complete_action();
			mutex_unlock(&elevator_state.lock);
		}
	}
	return 0;
}

int elevator_thread_init(void)
{
	elevator.deactivating = false;
	elevator.direction = ELEVATOR_UP;
	elevator.num_serviced = 0;

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
	mutex_lock(&elevator_state.lock);
	elevator_state.status = ELEVATOR_OFFLINE;
	mutex_unlock(&elevator_state.lock);
}

int elevator_activate(void)
{
	int ret = 0;

	mutex_lock(&elevator_state.lock);
	if (elevator_state.status != ELEVATOR_OFFLINE) {
		ret = 1;
	} else {
		elevator_state.status = ELEVATOR_IDLE;
		elevator_state.current_weight = 0;
		elevator_state.passenger_count = 0;
		elevator.direction = ELEVATOR_UP;
		elevator.deactivating = false;
	}
	mutex_unlock(&elevator_state.lock);
	return ret;
}

int elevator_deactivate(void)
{
	int ret = 0;

	mutex_lock(&elevator_state.lock);
	if (elevator_state.status == ELEVATOR_OFFLINE || elevator.deactivating)
		ret = 1;
	else
		elevator.deactivating = true;
	mutex_unlock(&elevator_state.lock);
	return ret;
}
