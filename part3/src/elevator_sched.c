#include <linux/slab.h>

#include "elevator.h"

static struct elevator_floor *current_floor(void)
{
	return &elevator_state.floors[elevator_state.current_floor - 1];
}

static bool has_unloading(void)
{
	struct elevator_pet *pet;

	list_for_each_entry(pet, &elevator_state.passengers, link)
		if (pet->destination_floor == elevator_state.current_floor)
			return true;
	return false;
}

static bool head_fits(struct elevator_floor *floor)
{
	struct elevator_pet *pet;

	if (elevator.deactivating || list_empty(&floor->waiting_pets))
		return false;
	pet = list_first_entry(&floor->waiting_pets, struct elevator_pet, link);
	return elevator_state.passenger_count < ELEVATOR_CAPACITY &&
	       elevator_state.current_weight + pet->weight <= ELEVATOR_WEIGHT_LIMIT;
}

static bool has_work(enum elevator_status dir)
{
	struct elevator_pet *pet;
	int floor;

	list_for_each_entry(pet, &elevator_state.passengers, link) {
		if (dir == ELEVATOR_UP &&
		    pet->destination_floor > elevator_state.current_floor)
			return true;
		if (dir == ELEVATOR_DOWN &&
		    pet->destination_floor < elevator_state.current_floor)
			return true;
	}

	if (elevator.deactivating)
		return false;

	for (floor = 1; floor <= ELEVATOR_FLOOR_COUNT; floor++) {
		if (!elevator_state.floors[floor - 1].waiting_count)
			continue;
		if (dir == ELEVATOR_UP && floor > elevator_state.current_floor)
			return true;
		if (dir == ELEVATOR_DOWN && floor < elevator_state.current_floor)
			return true;
	}
	return false;
}

static enum elevator_status opposite(enum elevator_status dir)
{
	return dir == ELEVATOR_UP ? ELEVATOR_DOWN : ELEVATOR_UP;
}

int elevator_next_action(void)
{
	if (has_unloading() || head_fits(current_floor())) {
		elevator_state.status = ELEVATOR_LOADING;
		return LOAD_DELAY_MS;
	}

	if (!has_work(elevator.direction)) {
		elevator.direction = opposite(elevator.direction);
		if (!has_work(elevator.direction))
			return 0;
	}

	elevator_state.status = elevator.direction;
	return MOVE_DELAY_MS;
}

static void unload_pets(void)
{
	struct elevator_pet *pet;
	struct elevator_pet *next;

	list_for_each_entry_safe(pet, next, &elevator_state.passengers, link) {
		if (pet->destination_floor != elevator_state.current_floor)
			continue;
		list_del(&pet->link);
		elevator_state.passenger_count--;
		elevator_state.current_weight -= pet->weight;
		elevator.num_serviced++;
		kfree(pet);
	}
}

static void load_pets(void)
{
	struct elevator_floor *floor = current_floor();
	struct elevator_pet *pet;

	while (head_fits(floor)) {
		pet = list_first_entry(&floor->waiting_pets,
				       struct elevator_pet, link);
		list_move_tail(&pet->link, &elevator_state.passengers);
		floor->waiting_count--;
		elevator_state.passenger_count++;
		elevator_state.current_weight += pet->weight;
	}
}

void elevator_complete_action(void)
{
	switch (elevator_state.status) {
	case ELEVATOR_UP:
		if (elevator_state.current_floor < ELEVATOR_FLOOR_COUNT)
			elevator_state.current_floor++;
		break;
	case ELEVATOR_DOWN:
		if (elevator_state.current_floor > 1)
			elevator_state.current_floor--;
		break;
	case ELEVATOR_LOADING:
		unload_pets();
		load_pets();
		break;
	default:
		break;
	}
}
