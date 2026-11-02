#!/usr/bin/env bash
#
# apply_syscalls.sh - add the elevator syscalls to a kernel source tree
#                     (Part 3a)
#
# Adding the system calls takes these changes to the kernel source, which
# follow the file layout the project spec asks for:
#
#   1. Create <ksrc>/syscalls/ with syscalls.c and a one-line Makefile
#      (the syscall code itself)
#   2. Add syscalls/ to the top-level Makefile (so kbuild visits that
#      directory and links the result into the kernel)
#   3. Add entries 548-550 to arch/x86/entry/syscalls/syscall_64.tbl
#      (gives each syscall a number; the build turns this table into the
#      sys_call_table array the CPU entry code looks up)
#   4. Add prototypes to include/linux/syscalls.h (the official
#      declaration of every sys_* function)
#
# Safe to re-run: each edit checks whether it's already there before
# changing anything. The first run backs up each edited file as *.orig,
# so you can compare with `diff file.orig file` or restore it.
#
# Usage:
#   sudo ./apply_syscalls.sh /usr/src/linux-<version>
#   (sudo only if the source tree belongs to root, which is typical for
#   /usr/src)
#
# Next step: ./build_kernel.sh <same path>

# Stop on the first error (-e), on undefined variables (-u), and when any
# command in a pipeline fails (-o pipefail).
set -euo pipefail

# Kernel source directory from the first argument. ${1:?msg} prints usage
# and exits if it's missing. realpath makes it absolute.
KSRC=$(realpath "${1:?usage: $0 <kernel-source-dir>}")

# This script's own directory, so ../syscalls.c resolves correctly no
# matter where the script is run from.
HERE=$(cd "$(dirname "$0")" && pwd)

# Files we edit.
TBL=$KSRC/arch/x86/entry/syscalls/syscall_64.tbl	# syscall number table
HDR=$KSRC/include/linux/syscalls.h			# syscall prototypes
TOP=$KSRC/Makefile					# top-level build file

# ---------------------------------------------------------------------
# Sanity check + backups
#   If any expected file is missing, the path probably isn't a kernel
#   tree, so stop before touching anything. Back up each file once (only
#   if no .orig exists yet, so re-runs never overwrite the clean copy).
# ---------------------------------------------------------------------
for f in "$TBL" "$HDR" "$TOP"; do
	[ -f "$f" ] || { echo "missing $f; is $KSRC a kernel tree?" >&2; exit 1; }
	[ -f "$f.orig" ] || cp "$f" "$f.orig"
done

# ---------------------------------------------------------------------
# Step 1: add the syscall code
#   Always overwrite, so changes to part3/syscalls.c reach the kernel tree
#   on every run. The Makefile tells kbuild to compile syscalls.c and link
#   it into the kernel image (y = built-in, unlike m = module).
# ---------------------------------------------------------------------
mkdir -p "$KSRC/syscalls"
cp "$HERE/../syscalls.c" "$KSRC/syscalls/syscalls.c"
echo 'obj-y := syscalls.o' > "$KSRC/syscalls/Makefile"
echo "created syscalls/syscalls.c and syscalls/Makefile"

# ---------------------------------------------------------------------
# Step 2: add syscalls/ to the kernel build
#   core-y lists extra directories to build into the kernel. In 6.16 the
#   top-level Makefile starts it empty ("core-y := "), the arch Makefile
#   adds to it, and the top-level Kbuild file builds whatever it holds.
#   Our line has to come right after that first assignment, or it would
#   be wiped out. Stop with an error if the line can't be found, since
#   the syscalls would otherwise be silently left out of the kernel.
# ---------------------------------------------------------------------
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

# ---------------------------------------------------------------------
# Step 3: give the syscalls numbers
#   Format: <number> <abi> <name> <entry point>
#     abi "common" = available to both 64-bit and x32 programs
#     entry point  = the sys_<name> function SYSCALL_DEFINEn creates
#   We use 548-550 because 512-547 are taken by x32 compatibility
#   syscalls, so 548 is the first free number past that range. Columns are
#   separated by tabs (the quoted heredoc keeps them as written).
# ---------------------------------------------------------------------
if ! grep -q 'sys_start_elevator' "$TBL"; then
	cat >> "$TBL" <<'TBL_EOF'
548	common	start_elevator		sys_start_elevator
549	common	issue_request		sys_issue_request
550	common	stop_elevator		sys_stop_elevator
TBL_EOF
	echo "added entries 548-550 to syscall_64.tbl"
fi

# ---------------------------------------------------------------------
# Step 4: declare the prototypes
#   syscalls.h is wrapped in an include guard (#ifndef ... #endif), so the
#   new lines must go BEFORE the last #endif or they'd be outside the
#   guard. Approach:
#     - find the line number n of the last "#endif"
#     - write lines 1..n-1, then our prototypes, then line n to the end,
#       into a temp file
#     - replace the original with the temp file
#   This avoids GNU-only sed multi-line insert syntax.
#   asmlinkage tells the compiler the arguments come the way the syscall
#   entry code passes them.
# ---------------------------------------------------------------------
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
