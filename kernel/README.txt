==================================
SUMMARY
==================================

The smp.c file contained here is from the Raspberry PI OS version 5.10.17-v7l.
Hopefully the smp.c.diff will continue to apply on other versions as well.

After building the kernel (don't install yet), as described below, then:
- patch -p1 < smp.c.diff
- make -j4
- run the Install steps from the instructions attached below
- add 'maxcpus=N' to /proc/cmdline.txt  (N=3 for Raspberry Pi 4).
- reboot

After reboot, confirm new OS is running by:
- uname -a
- cat /proc/kallsyms | grep run_offline_cpu

Next build and run the module.
View module output in dmesg, or /var/log/messages

==================================
RASPBERRY PI KERNEL BUILD INSTRUCTIONS
==================================

Instructions copied below are from:
   https://www.raspberrypi.org/documentation/linux/kernel/building.md

Install build tools
  sudo apt install git bc bison flex libssl-dev make

Clone kernel src
  git clone --depth=1 https://github.com/raspberrypi/linux

Configure for raspberry pi 4, default build config
  cd linux
  KERNEL=kernel7l
  make bcm2711_defconfig

In addition to your kernel configuration changes, you may wish to adjust the 
LOCALVERSION to ensure your new kernel does not receive the same version string 
as the upstream kernel. This both clarifies you are running your own kernel in the 
output of uname and ensures existing modules in /lib/modules are not overwritten.

To do so, change the following line in .config:
  CONFIG_LOCALVERSION="-v7l-MY_CUSTOM_KERNEL"

Build
  make -j4 zImage modules dtbs

Install
  sudo make modules_install
  sudo cp arch/arm/boot/dts/*.dtb /boot/
  sudo cp arch/arm/boot/dts/overlays/*.dtb* /boot/overlays/
  sudo cp arch/arm/boot/dts/overlays/README /boot/overlays/
  sudo cp arch/arm/boot/zImage /boot/$KERNEL.img

Another option is to copy the kernel into the same place, but with a different 
filename - for instance, kernel-myconfig.img - rather than overwriting the kernel.img file. 
You can then edit the config.txt file to select the kernel that the Pi will boot into:

==================================
BACKUP INFO - CPU HOTPLUG
==================================

I first investigated CPU Hot Plug, and built a kernel with CONFIG_HOTPLUG_CPU=y. 
This was interesting, but ultimately I did not make use of cpu hotplug, because it brings
the CPU into Linux. 

Hot Plug CPU Documentation 
- Documentation/core-api/cpu_hotplug.rst

Kernel cmdline option:
- maxcpus

Interesting routines found when investigating hot plug cpu.
- __cpu_up
   - architecture specific cpu bringup
   - ret = smp_ops.smp_boot_secondary(cpu, idle);
- secondary_startup
   - new cpu starts here, in assembly code
   - calls C code secondary_start_kernel
- cpuhp_hp_states
   - in file kernel/cpu.c
   - these are the steps to bring a cpu up, and into Linux,
   - one of the steps is bringup_cpu(), which calls __cpu_up

/sys/devices/system/cpu
   - hotplug/states
   - cpu3/hotplug/state
   - cpu3/hotplug/target   <== for linux cpu hotplug, can set this to the desired state
                               to bringup a cpu; for example:
                               echo 216 > /sys/devices/system/cpu/cpu3/hotplug/target
