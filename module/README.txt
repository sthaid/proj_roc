=======================================
SUMMARY
=======================================

First follow the instructions in the kernel directory, to:
- build patched kernel
- install this kernel
- set maxcpus to keep the highest number cpu offline
- reboot
- confirm the new kernel is running

Next, in this directory:
- make
- make install
- make load
- tail -f /var/log/messages
- make unload

