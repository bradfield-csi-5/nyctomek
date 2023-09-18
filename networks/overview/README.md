# PCapture the Flag

## The pcap File Format

### The Global `pcap-savefile` Header

**Question**
- What’s the magic number? What does it tell you about the byte ordering in the pcap-specific aspects of the file?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 4 -C net.cap
00000000  d4 c3 b2 a1                                       |....|
00000004
```
The magic number is `0xa1b2c3d4` in Little Endian.
That means the pcap-specific parts of the file will be Little Endian.

**Question**
- What are the major and minor versions? Don’t forget about the byte ordering!

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 2 -s 4 -C net.cap
00000004  02 00                                             |..|
00000006
```
Major version is 2.

```
[tomasz.tomasz-laptop]> hexdump -n 2 -s 6 -C net.cap
00000006  04 00                                             |..|
00000008
```
Minor version is 4.

**Question**
- Are the values that ought to be zero in fact zero?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 8 -s 8 -C net.cap
00000008  00 00 00 00 00 00 00 00                           |........|
00000010
```
Yes, the fields Reserved1 and Reserved2 are zeros.

**Question**
- What is the snapshot length?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 4 -s 16 -C net.cap
00000010  ea 05 00 00                                       |....|
00000014

[tomasz.tomasz-laptop]> python3 -c 'print(int("0x5ea", 16))'
1514
```
The snapsnot length is 1514.

**Question**
- What is the link layer header type?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 2 -s 20 -C net.cap
00000014  01 00                                             |..|
00000016
```
The link layer header type is 1, which is `LINKTYPE_ETHERNET` according to the [documentation](https://www.tcpdump.org/linktypes.html).

### Per-packet Headers

**Question**
- What is the size of the first packet?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 4 -s 32 -C net.cap
00000020  4e 00 00 00                                       |N...|
00000024

[tomasz.tomasz-laptop]> python3 -c 'print(int("0x4e", 16))'
78
```
The size of the first packet is 78 bytes.

**Question**
- Was any data truncated?

**Answer**
```
[tomasz.tomasz-laptop]> hexdump -n 4 -s 36 -C net.cap
00000024  4e 00 00 00                                       |N...|
00000028
```
The un-truncated packet length is also 0x4e, or 78 bytes, meaning that no data was truncated.

### Parsing the Ethernet Headers

**Question**
- As a stretch goal, you may also want to confirm that the [organizationally unique identifiers](https://en.wikipedia.org/wiki/Organizationally_unique_identifier) math your expectations.

**Answer**
- The OUIs are the first 3 bytes of each MAC address.  There are two MAC addresses in the capture:
  - `c4:e9:84:87:60:28`, its OUI belongs to [TP-LINK](https://hwaddress.com/oui-iab/C4-E9-84/).
  - `a4:5e:60:df:2e:1b`, its OUI belongs to [Apple, Inc.](https://hwaddress.com/oui-iab/A4-5E-60/).
- These OUIs match expectations since the packets were probably captured between an Apple device, such as a MacBook and a router.

## Summary

- See [parsecap.c](parsecap.c) for the code that parsed the [provide capture file](net.cap).
- The stdout of a parsecap.c run on the [provide capture file](net.cap) is in [output.txt](output.txt).
- The reconstructed payload is an image of [Vint Cerf](https://en.wikipedia.org/wiki/Vint_Cerf):

  <img src=image.jpg>
