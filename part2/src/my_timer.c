#include <linux/errno.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/timekeeping.h>

#define PROC_NAME "timer"

static struct proc_dir_entry *timer_entry;
static DEFINE_MUTEX(timer_lock);
static struct timespec64 previous_time;
static bool previous_time_valid;

static int timer_show(struct seq_file *seq, void *unused)
{
	struct timespec64 now;
	struct timespec64 elapsed;

	mutex_lock(&timer_lock);
	ktime_get_real_ts64(&now);

	seq_printf(seq, "current time: %lld.%09lu\n",
		   (long long)now.tv_sec, (unsigned long)now.tv_nsec);

	if (previous_time_valid) {
		elapsed.tv_sec = now.tv_sec - previous_time.tv_sec;
		elapsed.tv_nsec = now.tv_nsec - previous_time.tv_nsec;
		if (elapsed.tv_nsec < 0) {
			elapsed.tv_sec--;
			elapsed.tv_nsec += NSEC_PER_SEC;
		}

		seq_printf(seq, "elapsed time: %lld.%09lu\n",
			   (long long)elapsed.tv_sec,
			   (unsigned long)elapsed.tv_nsec);
	}

	previous_time = now;
	previous_time_valid = true;
	mutex_unlock(&timer_lock);
	return 0;
}

static int timer_open(struct inode *inode, struct file *file)
{
	return single_open(file, timer_show, NULL);
}

static const struct proc_ops timer_proc_ops = {
	.proc_open = timer_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int __init my_timer_init(void)
{
	timer_entry = proc_create(PROC_NAME, 0444, NULL, &timer_proc_ops);
	if (!timer_entry) {
		pr_err("my_timer: unable to create /proc/%s\n", PROC_NAME);
		return -ENOMEM;
	}

	pr_info("my_timer: loaded; read /proc/%s for timestamps\n", PROC_NAME);
	return 0;
}

static void __exit my_timer_exit(void)
{
	proc_remove(timer_entry);
	pr_info("my_timer: unloaded; /proc/%s removed\n", PROC_NAME);
}

module_init(my_timer_init);
module_exit(my_timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ermithe");
MODULE_DESCRIPTION("Report current and elapsed real time through /proc/timer");
