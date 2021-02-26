# proj_roc
Run code on an offline CPU using Raspberry Pi OS.

The goal of this code is to provide an improved real time capability in the Raspberry Pi OS.

Techniques are available to run real-time code on Linux, these include:
* use 'isolcpus' on the kernel cmdline, and sched_setaffinity
* use sched_setscheduler to select real-time policy such as SCHED_FIFO
* there may also be ways to redirect interrupts to other CPUs

These are good techniques, but limitations include:
* The migration process runs at the highest real-time priority, and has a thread for each CPU.
* Interrupts still occur on the isolated processor.
* Linux needs some time every second to run other stuff (real time throttling). This is adjustable, but changing this behavior risks causing problems. The default is that real-time code can run for up to 950 ms every second. 
* The Real Time Throttling problem can be solved if you add periodic short delays, such as nanosleep(1000 ns). However, requesting 1 us delay, only guarantees that the delay will be at least 1 us, and will probably be longer. My experimentation on a Raspberry Pi 4, is the sleep time ranges from 1 - 50 us, with the majority at 20 us.

This project provides the ability to activate on offline CPU, but without incorporating this CPU into Linux. And run code of your choosing on this CPU. Summary of the execution environment:
* Can access the Linux kernel address space.
* Interrupts are disabled.
* Can not call most Linux kernel routines, although some, such as sprintf, may be okay.
* A short delay can be achieved by polling the hardware timer.
* Access to hardware registers must be pre-allocated by the module.
* One, or more, offline CPUs must be available. This is achieved by using 'maxcpus=N' on the kernel cmdline.

To make use of this, you will need to:
* Build a custom kernel, refer to the kernel directory. This kernel provides export routine 'run_offline_cpu(cpu, proc)'.
* Build a kernel module that contains the code you want to run on the offline CPU. See module directory for example.

Results are very good, and are described in comments in module/roc.c.
