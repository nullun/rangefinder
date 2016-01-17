RangeFinder
===========

This project started to help identify the range of a network on any given NIC port. Using nothing but a small PIC controlled device and some form of output, the user would be able to determine the range of packets traversing the network.

Requirements
------------

 * Listen for ANY addressed packet (Promiscuous)
 * Avoid using pcap for portability
 * Indicate the range either in full (LCD) or a predefined value (LED)

TODO
----

 * Choose a license
 * Port code over to PIC [TCP/IP stack](http://www.microchip.com/tcpip/)
 * Separate functions out into other files
 * Rewrite the signal interrupt function
 * Design schematic diagram
 * Fix various text output bugs

Usage
-----

Linux:

```
sudo ./rangefinder
```

References
----------

[C Language Examples of IPv4 and IPv6 Raw Sockets for Linux](http://www.pdbuchan.com/rawsock/rawsock.html)

[socket(2) - Linux man page](http://linux.die.net/man/2/socket)

Contributing
------------
nullun <git@nullun.com>

