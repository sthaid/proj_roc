=======================================
MODULE BUILD, INSTALL, TEST PROCEDURE
=======================================

First follow the instructions in the kernel directory, to:
- build a modified kernel
- install this kernel
- set maxcpus to keep the highest number cpu offline
- reboot
- confirm the new kernel is running, and the highest number cpu is offline

Next, in this directory ...

Review the roc.c code and Makefile

Build, Install and Test
- make
- make install
- make load
- tail -f /var/log/messages
- make unload

