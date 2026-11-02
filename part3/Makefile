obj-m += elevator.o
elevator-objs := src/elevator_module.o src/elevator_thread.o src/elevator_sched.o \
		src/pet_queues_mutex.o src/elevator_proc.o src/elevator_syscalls.o

ccflags-y := -I$(src)/src

KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all clean

all:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean
