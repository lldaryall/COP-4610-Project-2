#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#define __NR_START_ELEVATOR 548
#define __NR_ISSUE_REQUEST  549
#define __NR_STOP_ELEVATOR  550

static long report(const char *name, long ret)
{
	if (ret < 0)
		printf("%-28s -> %ld (%s)\n", name, ret, strerror(errno));
	else
		printf("%-28s -> %ld\n", name, ret);
	return ret;
}

static long do_start(void)
{
	return report("start_elevator()", syscall(__NR_START_ELEVATOR));
}

static long do_issue(int s, int d, int t)
{
	char label[64];

	snprintf(label, sizeof(label), "issue_request(%d, %d, %d)", s, d, t);
	return report(label, syscall(__NR_ISSUE_REQUEST, s, d, t));
}

static long do_stop(void)
{
	return report("stop_elevator()", syscall(__NR_STOP_ELEVATOR));
}

/* needs elevator.ko loaded and OFFLINE */
static int smoke(void)
{
	int fail = 0;

	fail |= do_start() != 0;
	fail |= do_start() != 1;

	fail |= do_issue(1, 5, 0) != 0;
	fail |= do_issue(5, 1, 3) != 0;

	fail |= do_issue(0, 3, 1) != 1;
	fail |= do_issue(2, 6, 1) != 1;
	fail |= do_issue(2, 3, 4) != 1;
	fail |= do_issue(2, 3, -1) != 1;
	fail |= do_issue(3, 3, 1) != 1;

	fail |= do_stop() != 0;

	printf("%s\n", fail ? "SMOKE TEST FAILED" : "SMOKE TEST PASSED");
	return fail;
}

int main(int argc, char **argv)
{
	if (argc == 2 && strcmp(argv[1], "start") == 0)
		return do_start() < 0;
	if (argc == 5 && strcmp(argv[1], "issue") == 0)
		return do_issue(atoi(argv[2]), atoi(argv[3]), atoi(argv[4])) < 0;
	if (argc == 2 && strcmp(argv[1], "stop") == 0)
		return do_stop() < 0;
	if (argc == 2 && strcmp(argv[1], "smoke") == 0)
		return smoke();

	fprintf(stderr,
		"usage: %s start | stop | smoke | issue <start> <dest> <type>\n",
		argv[0]);
	return 2;
}
