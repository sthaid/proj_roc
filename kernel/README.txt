==================================
KERNEL BUILD & INSTALL PROCEDURE
==================================

The smp.c file contained here is oringates from the Raspberry PI OS version 5.10.17-v7l;
and is modified to add the run_offline_cpu API.

Hopefully the smp.c.diff will continue to apply on other versions as well.

The following instructions are derived from:
https://www.raspberrypi.org/documentation/linux/kernel/building.md

  # install tools
  sudo apt install git bc bison flex libssl-dev make

  # clone the kernel
  git clone --depth=1 https://github.com/raspberrypi/linux
  cd linux

  # apply the patch to add roc_offline_cpu() to arch/arm/kernel/x86.c
  patch -p1 < PATHNAME/smp.c.diff

  # make .config and update it
  KERNEL=kernel7l
  make bcm2711_defconfig
  vi .config   # and set 
     CONFIG_LOCALVERSION="-v7l-MY_CUSTOM_KERNEL"
     CONFIG_HOTPLUG_CPU=y

  # build
  make -j4 zImage modules dtbs
     Answer [N] (the default answer) to the following:
     Enable CPU hotplug state control (CPU_HOTPLUG_STATE_CONTROL) [N/y/?] (NEW) 

  # install
  sudo make modules_install
  sudo cp arch/arm/boot/dts/*.dtb /boot/
  sudo cp arch/arm/boot/dts/overlays/*.dtb* /boot/overlays/
  sudo cp arch/arm/boot/dts/overlays/README /boot/overlays/
  sudo cp arch/arm/boot/zImage /boot/$KERNEL.img

  # change maxcpus, to reserve a cpu
  vi /boot/cmdline.txt  # add the following, where N=3 for Raspberry Pi 4
     max_cpus=N

  # reboot
  reboot

  # after reboot verify new kernel is running
  uname -a
  cat /proc/kallsyms | grep run_offline_cpu

  # verify the correct CPU is offline
  lscpu

Next go to the module directory and build and run the module.

==================================
BACKUP INFO - RASPBERRY PI KERNEL BUILD INSTRUCTIONS
==================================

Instructions shown below are from:
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
BACKUP INFO - CPU HOTPLUG NOTES
==================================

I first investigated CPU Hot Plug, and built a kernel with CONFIG_HOTPLUG_CPU=y. 

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
