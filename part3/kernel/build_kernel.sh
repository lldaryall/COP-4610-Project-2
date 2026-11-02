#!/usr/bin/env bash
# usage: ./build_kernel.sh /usr/src/linux-<version>   (env: JOBS=N, LOCALMODCONFIG=0)
set -euo pipefail

KSRC=$(realpath "${1:?usage: $0 <kernel-source-dir>}")
JOBS=${JOBS:-$(nproc)}
LOCALMODCONFIG=${LOCALMODCONFIG:-1}

cd "$KSRC"

if [ ! -f .config ]; then
	echo "seeding .config from /boot/config-$(uname -r)"
	cp "/boot/config-$(uname -r)" .config
	if [ "$LOCALMODCONFIG" = 1 ]; then
		make localmodconfig < <(yes '')
	fi
fi

scripts/config --set-str SYSTEM_TRUSTED_KEYS ""
scripts/config --set-str SYSTEM_REVOCATION_KEYS ""
make olddefconfig

make -j"$JOBS"

sudo make modules_install
sudo make install
command -v update-grub >/dev/null && sudo update-grub

echo
echo "installed $(make -s kernelrelease). reboot, pick it in GRUB, then check:"
echo "  uname -r"
echo "  grep -E 'start_elevator|issue_request|stop_elevator' /proc/kallsyms"
