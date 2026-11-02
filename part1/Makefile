CC     = gcc
CFLAGS = -Wall -Wextra -O0

all: empty part1

empty: empty.c
	$(CC) $(CFLAGS) -o $@ $<

part1: part1.c
	$(CC) $(CFLAGS) -o $@ $<

traces: empty part1
	strace -o empty.trace ./empty
	strace -o part1.trace ./part1

check: traces
	@e=$$(wc -l < empty.trace); p=$$(wc -l < part1.trace); \
	echo "empty: $$e lines, part1: $$p lines, difference: $$((p - e))"; \
	test $$((p - e)) -eq 5 && echo "PASS" || (echo "FAIL: expected 5"; exit 1)

clean:
	rm -f empty part1

.PHONY: all traces check clean
