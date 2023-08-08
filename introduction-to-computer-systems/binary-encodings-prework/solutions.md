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

## 3.2 TCP

```
$ xxd tcpheader
00000000: af00 bc06 441e 7368 eff2 a002 81ff 5600  ....D.sh......V.

Source port      = af00 = a * 16^3 + f * 16^2 = 10 * 4096 + 15 * 256 = 44800.
Destination port = bc06 = b * 16^3 + c * 16^2 + 6 = (11 * 4096 + 12 * 156 + 6) = 46934.
Sequence number  = 441e7368 = 4 * 16^7 + 4 * 16^6 + 1 * 16^5 + e * 16^4 + 7 * 16^3 + 3 * 16^2 + 6 * 16^1 + 8 * 16^0 = 1142846312.
Data Offset      = 8 => we have 3 32-bit words (12 bytes) of optional fields.
```

## 3.3. Bonus: Byte ordering and integer encoding bitmaps

```

$ hexdump image1.bmp
0000000 42 4d 0a 0e 00 00 00 00 00 00 8a 00 00 00 7c 00
0000010 00 00 18 00 00 00 30 00 00 00 01 00 18 00 00 00
...

$ hexdump image2.bmp
0000000 42 4d 8a 18 00 00 00 00 00 00 8a 00 00 00 7c 00
0000010 00 00 20 00 00 00 40 00 00 00 01 00 18 00 00 00
...

image1.bmp
Size: 0x00000e0a = e * 16^2 + 10 = 14 * 16^2 + 10 = 3594 bytes.

image2.bmp
Size: 0x0000188a = 10 + 8 * 16^1 + 8 * 16^2 + 1 * 16^3 = 4096 + 8 * 256 + 128 + 10 = 6282 bytes.

There are a number of BMP format variants.  Which one is these?  (They are both the same variant.)
Both files have 7c 00 at byte offset 14.  Little Endian that's 0x007c = 124.
Therefore these are the BITMAPV5HEADER variant.

What are their respective dimensions?

$ hexdump image1.bmp
0000000 42 4d 0a 0e 00 00 00 00 00 00 8a 00 00 00 7c 00
0000010 00 00 18 00 00 00 30 00 00 00 01 00 18 00 00 00
              ^^^^^^^^^^^ ^^^^^^^^^^^
                  width      height
00000020  00 00 80 0d 00 00 00 00  00 00 00 00 00 00 00 00
                ^^^^^
              image size

width  = 0x00000018 = 24 pixels.
height = 0x00000030 = 48 pixels.

Calculating image size for fun:
24 bits/pixel * 24 pixels/row = 576 bits/row => 72 bytes.
72 bytes/row * 48 rows (i.e. height) = 3456 bytes image size.
3456 bytes ties out with the image size field: 80 0d (Little Endian).

$ hexdump image2.bmp
0000000 42 4d 8a 18 00 00 00 00 00 00 8a 00 00 00 7c 00
0000010 00 00 20 00 00 00 40 00 00 00 01 00 18 00 00 00
              ^^^^^^^^^^^ ^^^^^^^^^^^
                 width      height
00000020  00 00 00 18 00 00 00 00  00 00 00 00 00 00 00 00
                ^^^^^
              image size

width  = 0x00000020 = 32 pixels
height = 0x00000040 = 64 pixels.

Calculating image size for fun:
24 bits/pixel * 32 pixels/row = 768 bits/row => 96 bytes.
96 bytes/row * 64 rows (i.e. height) = 6144 bytes.
6144 bytes ties out with the image size field: 00 18 (Little Endian).
```
How much space is required to store each pixel?

```
$ hexdump image1.bmp
0000000 42 4d 0a 0e 00 00 00 00 00 00 8a 00 00 00 7c 00
0000010 00 00 18 00 00 00 30 00 00 00 01 00 18 00 00 00
                                            ^ 24 bits per pixel.
```
24 bits per pixel means 3 bytes are required to store each pixel.

Where does the data start?

```
The 4 bytes starting at offset 10 of the header tells us where the image data starts.  In each file, that's "8a 00 00 00", which is 138 in Little Endian.
```

What are the contents of each of the files?

image1.bmp has 0xffffff for all its image data, so it's all white.  Given its dimensions are 24x48 pixels, it's a 24x48 pixel white rectangle.

image2.bmp has 0x0000ff for all its image data.  According to the spec, the colors are ordered blue, green and red (not RGB!) so it's all red.  Given its dimensions are 32x64 pixels, it's a 32x64 pixel red rectangle.

