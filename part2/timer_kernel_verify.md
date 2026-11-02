## Part 2: My Build and Verification Record

## Environment

- Linux host: `offsec00`
- Kernel: `7.0.0-31-generic`
- Linux directory: `~/part2`

## Build

- Command: `make`
- Result: I built the module successfully; `my_timer.ko` was generated.
- Compiler: GCC 13.3.0
- Build notes: Kbuild printed a compiler identity warning. BTF generation was
  skipped because `vmlinux` was unavailable, but the module still built
  successfully.

## Load, Read, and Unload

Commands run:

```bash
sudo insmod my_timer.ko
lsmod | grep my_timer
cat /proc/timer
sleep 1
cat /proc/timer
sudo rmmod my_timer
test ! -e /proc/timer && echo "PASS: /proc/timer removed"
```

My results:

- I loaded the module successfully with `insmod`.
- `lsmod` listed `my_timer` (size `12288`).
- My first read showed `current time: 1793567941.202583212`.
- My second read showed `current time: 1793567963.266653513`.
- The module reported `elapsed time: 22.064070301`.
- I unloaded the module successfully with `rmmod`.
- The final check printed `PASS: /proc/timer removed`.

The elapsed interval between my reads was about 22.064 seconds, although I ran
`sleep 1` between them. I recorded the interval reported by the module.

## Verification Scope

I verified the build and the module's load, read, and unload behavior. I did not
run additional tests as part of this verification.
