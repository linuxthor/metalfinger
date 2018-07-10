# metalfinger

Linux kernel module to run a userspace application when a magic packet arrives.

Nothing needs to actually listen on the port in the traditional sense as we intercept early in the processing chain. 

Illustrates dev_add_pack() to add a 'sniffer' in the kernel, work queues, call_usermodehelper() (ugh..)  

```
make
insmod ./metalfinger.ko
hping3 -V -S -p 443 -s 42000 -w 128 127.0.0.1
ps faux
```
