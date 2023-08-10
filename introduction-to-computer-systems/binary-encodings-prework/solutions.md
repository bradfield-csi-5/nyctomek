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

You negate each bit and increment the value by 1.  We can compute subtraction a - b by adding a + (negation of b).

What is the value of the most significant bit in 8-bit two's complement?  What about 32-bit two's complement?

8-bit two's complement:
-(2^7) if the bit is 1, 0 otherwise.
32-bit two's complement:
-2(2^31) if the bit is 1, 0 otherwise.

## 2.5 Advanced: Integer Overflow Detection
It can be beneficial for our hardware to be able to detect overflow in two’s complement. To do so, we’d need a rule for determining—based solely on bit patterns—if overflow has occurred. Can you describe such a rule?

We look at the sign bits of the two values being added.  If their sign bits are equal but the sign bit of the sum is different, then overflow has occurred.  If the sign bits of the two numbers being added are different, then there can be no overflow.

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

# 4.  IEEE Floating Point

## 4.1 Deconstruction

Identify the 3 components of this 32-bit IEEE Floating Point Number and their values.
```
01000010001010100000000000000000

Using the three components, compute the value this represents.

01000010001010100000000000000000
^ sign bit is zero

01000010001010100000000000000000
 ^^^^^^^^
 Exponent is 10000100 = 132.

 Bias is (2^7)-1 = 127.

 So exponent is 5.  Therefore the number is (-1)^0 * M * 2^5.

 Solving for M:

 This is the normalized range, so M = 1 + f.

 0 10000100 01010100000000000000000
            ^^^^^^^^^^^^^^^^^^^^^^^
            f = b0.01010100000000000000000
              = 2^-2 + 2^-4 + 2^-6 = 1/4 + 1/16 + 1/64 = 16/64 + 4/64 + 1/64 = 21/64.

=> M = 1 21/64 => 85/64.

=> value is 1 * 85/64 * 32 = 1 * 85/2 = 85/2 = 42.5.
```

For the largest fixed exponent, 11111110 == 254 - 127 = 127, what is the smallest (magnitude) incremental change that can be made to a number?

The smallest change would be the LSB of the fractional part (23rd bit) flipping.
The 23rd bit would contribute 2^-23.  Since that would be multiplied by 2^127, the smallest incremental change would be 2^104 = ~2.03e31.

What does this imply about the precision of IEEE Floating Point values?

It implies the increments get larger at the outer bounds.  In other words, precision goes down drastically as the numbers get larger!

## 4.2 Advanced: Float Casting

```
unsigned int const v; // Round this 32-bit value to the next highest power of 2
unsigned int r;       // Put the result here. (So v=3 -> r=4; v=8 -> r=8)

if (v > 1) 
{
  float f = (float)v;
  unsigned int const t = 1U << ((*(unsigned int *)&f >> 23) - 0x7f);
  r = t << (t < v);
}
else 
{
  r = 1;
}

Let's walk through an example: v = 7, we would expect r = 8 after the code runs.

float f = (float)v;
^ This line initializes f to the IEEE-754 representation of the number 7, which is:
0 1000001 11000000000000000000000.

Derivation:
0 1000001 11000000000000000000000
^ sign
0 1000001 11000000000000000000000
  ^^^^^^^
  exponent field = 129
0 1000001 11000000000000000000000
          ^^^^^^^^^^^^^^^^^^^^^^^
          fractional field = 0.11

That gives us a normalized floating point value of (-1)^0 * 2^(129-127) * (1 + 1/2 + 1/4) =

2^2 * 1.75 = 7.

Now, this line:

unsigned int const t = 1U << ((*(unsigned int *)&f >> 23) - 0x7f);

`(*(unsigned int *)&f >> 23)`
Shifts away the fractional field, leaving 0b1000001.

`- 0x7f`
Subtracts out 127, effectively biasing the exponent, leaving:
0b00000010 (i.e. 2)

`1U <<`
Left shifts 0b1 by 0b0000010 (i.e. 2), leaving:
0b100.

And so t becomes 0b100, or 4 in demical.

`r = t << (t < v);`
Left shifts t one more place iff t < v.  That doubles t from 4 to 8.  Boom!
```

# 5. Character encodings

Is there any additional space cost to encoding a purely ASCII document as UTF-8?
Answer.  No.  Each character will be encoded as a single byte, just like in ASCII.

What are the advantages and detriments of UTF-8 compared to another encoding for Unicode such as UTF-32?
Answer.  The advantages of UTF-8 over UTF-32 are that it is compatible with ASCII and doesn't waste space for characters that don't need 4 bytes.  The disadvantage is that it requires more complex decoding logic, since characters don't have a fixed width.

## 5.1 Snowman

I expect snowman.txt to be 4 bytes in size and contain e298830a.

## 5.2 Hello again hellohex
The first 5 bytes where "hello" in ASCII.

The character encoding apparently is UTF-8, because it has the grinning face emoji at the end, hex code f0 9f 98 80, which is Unicode code point U+1F600.  The contents of the file are "Hello world 😀\n".

## 5.3 Bonus: Ding ding ding!

```
printf "\7\7\7"
```