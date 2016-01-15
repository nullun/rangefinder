RangeFinder
===========

This project started to help identify the range of a network on any given NIC port. Using nothing but a small PIC controlled device and some form of output, the user would be able to determine the range of packets traversing the network.

Requirements
------------

 * Listen for ANY addressed packet
 * Indicate range either in full or by LED and a predefined value

TODO
----

 * Choose a license
 * Port code over to PIC [TCP/IP stack](http://www.microchip.com/tcpip/)
 * Separate functions out into other files
 * Design schematic diagram

Usage
-----

Linux:

```
sudo ./rangefinder
```

Contributing
------------
nullun <git@nullun.com>

