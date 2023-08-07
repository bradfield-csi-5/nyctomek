# 1 Hexadecimal

## 1.1 Simple Conversion

9  = 0 * 16 + 9

Answer: 0x9.

136 = 8 * 16 + 8
  8 = 0 * 16 + 8

Answer: 0x88.

247 = 15 * 16 + 7
 15 = 0  * 16 + 15 (15 = 0xF)

Answer: 0xF7.

## 1.2 CSS colors

Both the rgb and hex representation allow 256 distinct values of r, g, b, for a total of 256^3 = (2^8)^3 = 2^24 = 16,777,216 combinations.

Say hello to hellohex

Each hex digit can encode 4 bits, so two hex digits encode one byte.  So for a 17 byte file I would expect 34 hex digits.

Confirmation:

```
$ let digits="$(xxd -p hellohex | wc -c)-1" && echo $digits
34
```

First 5 bytes:
0x68656c6c6f

Converted to binary:
```
   6    8    6    5    6    c    6    c    6    f
0110 1000 0110 0101 0110 1100 0110 1100 0110 1111
```
# Integers

## 2.1 Basic Conversion

Convert the following decimal numbers to binary.
```
4   0b100
65  0b1000001
105 0b1101001
255 0b11111111
```

Convert the following binary representations of unsigned integers to decimal.
```
10         2
11         3
1101100  108
1010101   85
```

## 2.2 Unsigned binary addition

```
  11111111 = 255
+ 00001101 =  13
----------
 100001100 = 2^2 + 2^3 + 2^8 = 4 + 8 + 256 = 268.
```
Answer: 11111111 + 00001101 = 100001100.

If registers are only 8 bits wide, the value returned is 00001100.  This is called integer overflow.

## 2.3 Two's complement conversion

Given the following decimal values, determine their 8 bit two's complement representations:
```
 127        0b01111111
-128        0b10000000
  -1        0b11111111
   1        0b00000001
 -14        0b11110010
```
Conver the following 8-bit two's complement numbers to decimal:
```
10000011    -(2^7) + 2^1 + 2^0 = -128 + 2 + 1 = -125
11000100    -(2^7) + 2^6 + 2^2 = -128 + 64 + 4 = -60
```
## 2.4 Addition of two's complement signed integers

What is the sum of the following two signed integers?  Does this match your expectations?

```
  01111111
+ 10000000
----------
  11111111
```
The sum is 0b11111111, which is -1.  It matches my expectations given that we're adding 127 (01111111) and -128 (10000000).

How do you negate a number in two's complement?  How can we compute subtraction of two's complement numbers?

__TODO__

What is the value of the most significant bit in 8-bit two's complement?  What about 32-bit two's complement?

8-bit two's complement:
-(2^7) if the bit is 1, 0 otherwise.
32-bit two's complement:
-2(2^31) if the bit is 1, 0 otherwise.

# 3 Byte Ordering

## 3.1 It's over 9000!

```
$ xxd 9001
00000000: 2329 

Let's convert 9001 to hex:

9001 = 562 * 16 + 9 # least significant digit
 562 =  35 * 16 + 2
  35 =   2 * 16 + 3
   2 =   0 * 16 + 2 # most significant digit

Therefore 9001 in hex is: 0x2329

That matches the xxd dump, which means the MSB was encoded first, which is a Big Endian byte ordering.
```