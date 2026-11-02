#include <unistd.h>

int main(void)
{
	static const char msg[] = "part1: making five system calls\n";

	if (write(STDOUT_FILENO, msg, sizeof(msg) - 1) < 0)
		return 1;

	getpid();
	getppid();
	getuid();
	getgid();

	return 0;
}
