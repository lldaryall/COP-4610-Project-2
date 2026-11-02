#!/usr/bin/env bash
#
# build_kernel.sh - configure, compile, and install the modified kernel
#                   (Part 3b)
#
# Run after apply_syscalls.sh. Steps:
#
#   1. CONFIGURE  make a .config (the list of features/drivers to build)
#   2. COMPILE    build the kernel image (bzImage) and its modules
#   3. INSTALL    copy modules to /lib/modules/<version>/, copy the kernel
#                 to /boot/, build an initramfs, and update GRUB
#   4. VERIFY     (after you reboot) confirm the new kernel is running and
#                 has our syscalls
#
# Usage:
#   ./build_kernel.sh /usr/src/linux-<version>
#   (the project uses the latest stable 6.16.x kernel)
#
# Options (environment variables):
#   JOBS=N            number of parallel compile jobs (default: number of
#                     CPUs). Lower it if the VM runs out of memory.
#   LOCALMODCONFIG=0  build every module from the distro config instead of
#                     only the ones loaded now. Much slower (hours vs.
#                     minutes); only needed if the trimmed kernel is
#                     missing a driver.
#
# Prerequisites (Ubuntu):
#   sudo apt install build-essential libncurses-dev bison flex libssl-dev \
#                    libelf-dev dwarves bc
#   (dwarves provides pahole, needed when CONFIG_DEBUG_INFO_BTF is on)

# Stop on the first error, undefined variable, or failed pipeline stage.
set -euo pipefail

# Kernel source directory (required), made absolute.
KSRC=$(realpath "${1:?usage: $0 <kernel-source-dir>}")

# ${VAR:-default}: use the environment variable if set, else the default.
JOBS=${JOBS:-$(nproc)}
LOCALMODCONFIG=${LOCALMODCONFIG:-1}

# Every kernel make target must run from the top of the source tree.
cd "$KSRC"

# ---------------------------------------------------------------------
# Step 1: CONFIGURE
#
# Only create .config if there isn't one, so re-runs (e.g. after fixing a
# compile error) keep your existing config and only rebuild what changed.
#
# Starting point: the config of the kernel running now, from /boot. It is
# known to boot on this VM, so a kernel built from it should too.
#
# localmodconfig then turns off every module that isn't loaded right now
# (from lsmod). A distro config builds thousands of drivers this VM never
# uses; skipping them cuts the build from hours to minutes. `yes ''`
# accepts the default answer for any new options it asks about.
# ---------------------------------------------------------------------
if [ ! -f .config ]; then
	echo "seeding .config from /boot/config-$(uname -r)"
	cp "/boot/config-$(uname -r)" .config
	if [ "$LOCALMODCONFIG" = 1 ]; then
		yes '' | make localmodconfig
	fi
fi

# Ubuntu's config tells the build to embed Canonical's signing
# certificates (debian/canonical-certs.pem), which aren't in a vanilla
# source tree. Without these two lines the build fails partway through
# with "No rule to make target 'debian/canonical-certs.pem'". Setting
# them to empty strings turns the feature off.
scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str SYSTEM_REVOCATION_KEYS ""

# Fill in any options the config doesn't mention (e.g. new features in
# this kernel version) with their defaults, without prompting.
make olddefconfig

# ---------------------------------------------------------------------
# Step 2: COMPILE
#
# Builds vmlinux/bzImage (which now includes syscalls/syscalls.o and the
# updated syscall table) plus every enabled module. -j runs that many
# compile jobs in parallel. If it fails, scroll up to the FIRST "error:"
# line; the last lines are usually just make reporting the failure.
# ---------------------------------------------------------------------
make -j"$JOBS"

# ---------------------------------------------------------------------
# Step 3: INSTALL (needs root)
#
# modules_install: copies the .ko files to /lib/modules/<version>/ and
#   creates the /lib/modules/<version>/build symlink back to this source
#   tree. part3/Makefile uses that symlink to build elevator.ko.
# install: copies bzImage to /boot/vmlinuz-<version>, along with
#   System.map and the config. On Ubuntu this also generates the
#   initramfs and usually updates GRUB.
# update-grub: regenerates the GRUB boot menu so the new kernel shows up.
#   Run explicitly in case `make install` didn't; skipped if the command
#   doesn't exist.
# ---------------------------------------------------------------------
sudo make modules_install
sudo make install
command -v update-grub >/dev/null && sudo update-grub

# ---------------------------------------------------------------------
# Step 4: VERIFY (manual, after reboot)
#
# `make -s kernelrelease` prints the version string just built (e.g.
# 6.10.0), which is what `uname -r` should show after reboot. kallsyms
# lists every kernel symbol; seeing our sys_* functions there confirms the
# syscalls made it into the running kernel.
# ---------------------------------------------------------------------
echo
echo "installed $(make -s kernelrelease). reboot, pick it in GRUB, then check:"
echo "  uname -r"
echo "  grep -E 'start_elevator|issue_request|stop_elevator' /proc/kallsyms"
