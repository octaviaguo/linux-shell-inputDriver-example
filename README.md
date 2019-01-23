# linux-shell-inputDriver-example
##Introduction
This project is a practice to help understand the kernel, driver device files and their interaction with userspace.
##Setup
In terminal, open the qemu with command (change the file paths to yours):
```
  qemu-system-x86_64     -kernel linux-4.10.6/arch/x86_64/boot/bzImage     -initrd obj/initramfsMykeyb.cpio.gz    -append "console=ttyS0" -serial stdio
```
Once the qemu is open, run the command in terminal to prepare for devices:
```
  mknod /dev/keyb c 61 0
  chmod 666 /dev/keyb
  mknod /dev/tty0 c 4 0
  chmod 666 /dev/tty0
```
Then open the shell with command in terminal:
```
  ./main
```
Now, you can use the devices -- in this case, a keyboard and a screen -- in qemu.
![](images/Keyb%20screenshot.png)
