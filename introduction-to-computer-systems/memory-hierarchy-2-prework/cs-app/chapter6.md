## Practice Problem 6.9

|Cache |  m   |   C   |   B  |   E  |   S   |   t   |    s   |    b   |
|------|------|-------|------|------|-------|-------|--------|--------|
|  1.  |  32  | 1024  |   4  |   1  |  256  |   22  |    8   |    2   |
|  2.  |  32  | 1024  |   8  |   4  |  32   |   24  |    5   |    3   |
|  3.  |  32  | 1024  |  32  |  32  |  1    |   27  |    0   |    5   |

## Practice Problem 6.10

3/4, or 75%

## Practice Problem 6.11

### A.
The chunks are 2^(m-s) bytes in size.
Blocks are 2^b bytes in size.

So there are: 2^(m-s) / 2^b

= 2^(m-s-b) blocks in each chunk.

Since t = m - (s + b) = m - s - b, this is equivalent to saying that there are 2^t blocks in each chunk.

### B.

With (S, E, B, m) = (512, 1, 32, 32), s = 9, which means the highest order 9 bits are used as the set index.

Because the array contains integers, it spans 4 * 4096 = 2^14 bytes of address space.  As we increment array[i], bits 0 - 13 of the address will change.  With m = 32, the upper 9 bits of the address are bits 23 - 31.  So as we move from array[0] to array[4096], none of the upper 9 bits of the address will change.

Therefore, every block of the array will be mapped to the same cache set.  Because each cache contains 1 cache line, at most 1 array block will be stored in the cache at any given point in time.

## Practice Problem 6.12

- CO bits 0 - 1
- CI bits 2 - 4
- CT bits 5 - 12

## Practice Problem 6.13

0x0E34 = 0b0111000110100

- Cache block offset (C0) = 00      = 0x0
- Cache set index (CI)    = 101     = 0x5
- Cache tag (CT)          = 1110001 = 0x71
- Cache hit? (Y/N)                  = Y
- Cache byte return                 = 0xB

## Practice Problem 6.14

0x0DD5 = 0b110111010101

- Cache block offset (C0) = 01      = 0x1
- Cache set index (CI)    = 101     = 0x5
- Cache tag (CT)          = 1101110 = 0x6e
- Cache hit? (Y/N)                  = N
- Cache byte return                 = --

## Practice Problem 6.15

0x1FE4 = 0b1111111100100

- Cache block offset (C0) = 00       = 0x0
- Cache set index (CI)    = 001      = 0x1
- Cache tag (CT)          = 11111111 = 0xFF
- Cache hit? (Y/N)                   = N
- Cache byte return                  = --

## Practice Problem 6.16

0b0011001001100 = 0x064c
0b0011001001101 = 0x064d
0b0011001001110 = 0x064e
0b0011001001111 = 0x064f

