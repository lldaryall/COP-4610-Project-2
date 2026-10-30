#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#include "elevator.h"

#define PROC_NAME "elevator"

static struct proc_dir_entry *elevator_entry;

static const char pet_letters[PET_TYPE_COUNT] = {
	[PET_CHIHUAHUA] = 'C',
	[PET_PUG] = 'P',
	[PET_PUGHUAHUA] = 'H',
	[PET_DACHSHUND] = 'D',
};

static void show_pet(struct seq_file *seq, struct elevator_pet *pet)
{
	seq_printf(seq, " %c%d", pet_letters[pet->type], pet->destination_floor);
}

static int elevator_show(struct seq_file *seq, void *unused)
{
	struct elevator_pet *pet;
	unsigned int waiting = 0;
	int floor;

	mutex_lock(&elevator_state.lock);

	seq_printf(seq, "Elevator state: %s\n",
		   elevator_state_name(elevator_state.status));
	seq_printf(seq, "Current floor: %d\n", elevator_state.current_floor);
	seq_printf(seq, "Current load: %d lbs\n", elevator_state.current_weight);
	seq_puts(seq, "Elevator status:");
	list_for_each_entry(pet, &elevator_state.passengers, link)
		show_pet(seq, pet);
	seq_puts(seq, "\n\n");

	for (floor = ELEVATOR_FLOOR_COUNT; floor >= 1; floor--) {
		struct elevator_floor *f = &elevator_state.floors[floor - 1];

		seq_printf(seq, "[%c] Floor %d: %u",
			   floor == elevator_state.current_floor ? '*' : ' ',
			   floor, f->waiting_count);
		list_for_each_entry(pet, &f->waiting_pets, link)
			show_pet(seq, pet);
		seq_putc(seq, '\n');
		waiting += f->waiting_count;
	}

	seq_printf(seq, "\nNumber of pets: %u\n", elevator_state.passenger_count);
	seq_printf(seq, "Number of pets waiting: %u\n", waiting);
	seq_printf(seq, "Number of pets serviced: %u\n", elevator.num_serviced);

	mutex_unlock(&elevator_state.lock);
	return 0;
}

static int elevator_open(struct inode *inode, struct file *file)
{
	return single_open(file, elevator_show, NULL);
}

static const struct proc_ops elevator_proc_ops = {
	.proc_open = elevator_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

int elevator_proc_init(void)
{
	elevator_entry = proc_create(PROC_NAME, 0444, NULL, &elevator_proc_ops);
	return elevator_entry ? 0 : -ENOMEM;
}

void elevator_proc_exit(void)
{
	proc_remove(elevator_entry);
}
