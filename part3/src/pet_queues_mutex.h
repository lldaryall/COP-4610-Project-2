/* Shared elevator state for the elevator module's thread, syscalls and procfs. */
#ifndef ELEVATOR_SHARED_H
#define ELEVATOR_SHARED_H

#include <linux/list.h>
#include <linux/mutex.h>

#define ELEVATOR_FLOOR_COUNT 5
#define ELEVATOR_CAPACITY 5
#define ELEVATOR_WEIGHT_LIMIT 50

enum elevator_status {
	ELEVATOR_OFFLINE,
	ELEVATOR_IDLE,
	ELEVATOR_LOADING,
	ELEVATOR_UP,
	ELEVATOR_DOWN,
};

enum pet_type {
	PET_CHIHUAHUA = 0,
	PET_PUG,
	PET_PUGHUAHUA,
	PET_DACHSHUND,
	PET_TYPE_COUNT,
};

struct elevator_pet {
	enum pet_type type;
	int start_floor;
	int destination_floor;
	int weight;
	struct list_head link;
};

struct elevator_floor {
	struct list_head waiting_pets;
	unsigned int waiting_count;
};

struct elevator_shared_state {
	/* Protects every field and every list in this structure. */
	struct mutex lock;
	struct elevator_floor floors[ELEVATOR_FLOOR_COUNT];
	struct list_head passengers;
	unsigned int passenger_count;
	int current_weight;
	int current_floor; /* Floors are numbered 1 through ELEVATOR_FLOOR_COUNT. */
	enum elevator_status status;
};

extern struct elevator_shared_state elevator_state;

/* Call during module initialization, before starting the elevator thread. */
void elevator_shared_init(void);

/* Queue one request at the tail of its start-floor FIFO. */
int elevator_queue_request(int start_floor, int destination_floor, int type);

/* Free every queued and onboard pet. Call after stopping the elevator thread. */
void elevator_shared_cleanup(void);

/* Pet names and weights, indexed by enum pet_type. */
const char *elevator_pet_name(enum pet_type type);
int elevator_pet_weight(enum pet_type type);

#endif /* ELEVATOR_SHARED_H */
