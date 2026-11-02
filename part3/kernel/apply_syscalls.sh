#!/usr/bin/env bash
# usage: sudo ./apply_syscalls.sh /usr/src/linux-<version>
set -euo pipefail

KSRC=$(realpath "${1:?usage: $0 <kernel-source-dir>}")
HERE=$(cd "$(dirname "$0")" && pwd)

TBL=$KSRC/arch/x86/entry/syscalls/syscall_64.tbl
HDR=$KSRC/include/linux/syscalls.h
TOP=$KSRC/Makefile

for f in "$TBL" "$HDR" "$TOP"; do
	[ -f "$f" ] || { echo "missing $f; is $KSRC a kernel tree?" >&2; exit 1; }
	[ -f "$f.orig" ] || cp "$f" "$f.orig"
done

mkdir -p "$KSRC/syscalls"
cp "$HERE/../syscalls.c" "$KSRC/syscalls/syscalls.c"
echo 'obj-y := syscalls.o' > "$KSRC/syscalls/Makefile"
echo "created syscalls/syscalls.c and syscalls/Makefile"

if ! grep -q '^core-y.*syscalls/' "$TOP"; then
	n=$(grep -n -E '^core-y[[:space:]]*:=' "$TOP" | head -1 | cut -d: -f1)
	[ -n "$n" ] || { echo "no 'core-y :=' line in $TOP" >&2; exit 1; }
	{
		head -n "$n" "$TOP"
		printf 'core-y\t\t+= syscalls/\n'
		tail -n +$((n + 1)) "$TOP"
	} > "$TOP.tmp"
	mv "$TOP.tmp" "$TOP"
	echo "added syscalls/ to the top-level Makefile"
fi

if ! grep -q 'sys_start_elevator' "$TBL"; then
	cat >> "$TBL" <<'TBL_EOF'
548	common	start_elevator		sys_start_elevator
549	common	issue_request		sys_issue_request
550	common	stop_elevator		sys_stop_elevator
TBL_EOF
	echo "added entries 548-550 to syscall_64.tbl"
fi

if ! grep -q 'sys_start_elevator' "$HDR"; then
	n=$(grep -n '^#endif' "$HDR" | tail -1 | cut -d: -f1)
	{
		head -n $((n - 1)) "$HDR"
		cat <<'HDR_EOF'
asmlinkage long sys_start_elevator(void);
asmlinkage long sys_issue_request(int start_floor, int destination_floor, int type);
asmlinkage long sys_stop_elevator(void);

HDR_EOF
		tail -n +"$n" "$HDR"
	} > "$HDR.tmp"
	mv "$HDR.tmp" "$HDR"
	echo "added prototypes to include/linux/syscalls.h"
fi

echo "done. next: ./build_kernel.sh $KSRC"
