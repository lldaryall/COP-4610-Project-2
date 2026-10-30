#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/slab.h>

#include "pet_queues_mutex.h"

struct elevator_shared_state elevator_state;

static const char * const pet_names[PET_TYPE_COUNT] = {
	[PET_CHIHUAHUA] = "Chihuahua",
	[PET_PUG] = "Pug",
	[PET_PUGHUAHUA] = "Pughuahua",
	[PET_DACHSHUND] = "Dachshund",
};

static const int pet_weights[PET_TYPE_COUNT] = {
	[PET_CHIHUAHUA] = 3,
	[PET_PUG] = 14,
	[PET_PUGHUAHUA] = 10,
	[PET_DACHSHUND] = 16,
};

const char *elevator_pet_name(enum pet_type type)
{
	if ((unsigned int)type >= PET_TYPE_COUNT)
		return "Unknown";
	return pet_names[type];
}

int elevator_pet_weight(enum pet_type type)
{
	if ((unsigned int)type >= PET_TYPE_COUNT)
		return -EINVAL;
	return pet_weights[type];
}

void elevator_shared_init(void)
{
	int floor;

	mutex_init(&elevator_state.lock);
	for (floor = 0; floor < ELEVATOR_FLOOR_COUNT; floor++) {
		INIT_LIST_HEAD(&elevator_state.floors[floor].waiting_pets);
		elevator_state.floors[floor].waiting_count = 0;
	}
	INIT_LIST_HEAD(&elevator_state.passengers);
	elevator_state.passenger_count = 0;
	elevator_state.current_weight = 0;
	elevator_state.current_floor = 1;
	elevator_state.status = ELEVATOR_OFFLINE;
}

int elevator_queue_request(int start_floor, int destination_floor, int type)
{
	struct elevator_pet *pet;
	int weight;

	if (start_floor < 1 || start_floor > ELEVATOR_FLOOR_COUNT ||
	    destination_floor < 1 || destination_floor > ELEVATOR_FLOOR_COUNT ||
	    start_floor == destination_floor ||
	    (unsigned int)type >= PET_TYPE_COUNT)
		return -EINVAL;

	weight = elevator_pet_weight(type);
	pet = kmalloc(sizeof(*pet), GFP_KERNEL);
	if (!pet)
		return -ENOMEM;

	pet->type = type;
	pet->start_floor = start_floor;
	pet->destination_floor = destination_floor;
	pet->weight = weight;

	mutex_lock(&elevator_state.lock);
	list_add_tail(&pet->link,
		      &elevator_state.floors[start_floor - 1].waiting_pets);
	elevator_state.floors[start_floor - 1].waiting_count++;
	mutex_unlock(&elevator_state.lock);

	return 0;
}

static void elevator_free_list(struct list_head *head)
{
	struct elevator_pet *pet;
	struct elevator_pet *next;

	list_for_each_entry_safe(pet, next, head, link) {
		list_del(&pet->link);
		kfree(pet);
	}
}

void elevator_shared_cleanup(void)
{
	int floor;

	mutex_lock(&elevator_state.lock);
	for (floor = 0; floor < ELEVATOR_FLOOR_COUNT; floor++) {
		elevator_free_list(&elevator_state.floors[floor].waiting_pets);
		elevator_state.floors[floor].waiting_count = 0;
	}
	elevator_free_list(&elevator_state.passengers);
	elevator_state.passenger_count = 0;
	elevator_state.current_weight = 0;
	mutex_unlock(&elevator_state.lock);

	mutex_destroy(&elevator_state.lock);
}



