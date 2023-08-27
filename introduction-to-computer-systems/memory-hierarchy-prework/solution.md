# Improving Cache Utilization

## 1.  Does Loop Order Matter?

### Which function takes longer to run?

`option_two` takes longer:
```
> time ./loop-order

real	0m0.287s
user	0m0.239s
sys	    0m0.048s
```

Compared to `option_one`:
```
> time ./loop-order

real	0m0.104s
user	0m0.068s
sys	    0m0.036s

```

`option_two` does a stride-N traversal, and therefore has much worse cache locality than `option_one`, which does a stride-1 traversal.

### Do they execute the same number of instructions?

According to `cachegrind`, they both execute the same number of instructions.  Specifically, 240,169,308.

`option_one`
```
> valgrind --tool=cachegrind ./loop-order
==11818== Cachegrind, a cache and branch-prediction profiler
==11818== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==11818== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==11818== Command: ./loop-order
==11818== 
--11818-- warning: L3 cache found, using its data for the LL simulation.
==11818== 
==11818== I   refs:      240,169,308
==11818== I1  misses:          1,126
==11818== LLi misses:          1,116
==11818== I1  miss rate:        0.00%
==11818== LLi miss rate:        0.00%
==11818== 
==11818== D   refs:      112,061,743  (96,045,244 rd   + 16,016,499 wr)
==11818== D1  misses:      1,002,177  (     1,588 rd   +  1,000,589 wr)
==11818== LLd misses:      1,001,902  (     1,349 rd   +  1,000,553 wr)
==11818== D1  miss rate:         0.9% (       0.0%     +        6.2%  )
==11818== LLd miss rate:         0.9% (       0.0%     +        6.2%  )
==11818== 
==11818== LL refs:         1,003,303  (     2,714 rd   +  1,000,589 wr)
==11818== LL misses:       1,003,018  (     2,465 rd   +  1,000,553 wr)
==11818== LL miss rate:          0.3% (       0.0%     +        6.2%  )
```

`option_two`
```
> valgrind --tool=cachegrind ./loop-order
==11852== Cachegrind, a cache and branch-prediction profiler
==11852== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==11852== Using Valgrind-3.18.1 and LibVEX; rerun with -h for copyright info
==11852== Command: ./loop-order
==11852== 
--11852-- warning: L3 cache found, using its data for the LL simulation.
==11852== 
==11852== I   refs:      240,169,308
==11852== I1  misses:          1,125
==11852== LLi misses:          1,115
==11852== I1  miss rate:        0.00%
==11852== LLi miss rate:        0.00%
==11852== 
==11852== D   refs:      112,061,743  (96,045,244 rd   + 16,016,499 wr)
==11852== D1  misses:     16,002,177  (     1,588 rd   + 16,000,589 wr)
==11852== LLd misses:      1,001,902  (     1,349 rd   +  1,000,553 wr)
==11852== D1  miss rate:        14.3% (       0.0%     +       99.9%  )
==11852== LLd miss rate:         0.9% (       0.0%     +        6.2%  )
==11852== 
==11852== LL refs:        16,003,302  (     2,713 rd   + 16,000,589 wr)
==11852== LL misses:       1,003,017  (     2,464 rd   +  1,000,553 wr)
==11852== LL miss rate:          0.3% (       0.0%     +        6.2%  )
```

### What do you notice about the cache utilization of each program? How much better is one than the other? Does this match your expectations?

`option_one` misses the cache once for every 16 write.
```
==11818== D   refs:      112,061,743  (96,045,244 rd   + 16,016,499 wr)
==11818== D1  misses:      1,002,177  (     1,588 rd   +  1,000,589 wr)
==11818== D1  miss rate:         0.9% (       0.0%     +        6.2%  )
```
That lines up with expectations.  My test machine has 64-byte cache lines.
```
[tomasz.tomasz-laptop]> getconf -a | grep LEVEL1_DCACHE.*SIZE
LEVEL1_DCACHE_SIZE                 32768
LEVEL1_DCACHE_LINESIZE             64
```
So with a stride-1 traversal of 4-byte integers, each cache line will hold 16 elements, so we'll have have a cache miss once out of every 16 accesses.

`option_two` misses the cache almost every single write.
```
==11852== D   refs:      112,061,743  (96,045,244 rd   + 16,016,499 wr)
==11852== D1  misses:     16,002,177  (     1,588 rd   + 16,000,589 wr)
==11852== LLd misses:      1,001,902  (     1,349 rd   +  1,000,553 wr)
```
That also lines up with expectations.  `option_two` was doing a stride-N traversal, N=4000, so it was jumping ~16KB away with every write, so the 64-byte cache lines weren't helping prevent cache misses at all.

- I have a theory that if N is reduced so much that `x` fits completely into my 32 KB L1 cache, `option_one` and `option_two` will exhibit similar cache utilization.

Testing out the theory:

`option_one`
```
[tomasz.tomasz-laptop]> valgrind --tool=cachegrind ./loop-order-small
...
==14411== D   refs:      1,797,742  (1,534,743 rd   + 262,999 wr)
==14411== D1  misses:       17,807  (    1,588 rd   +  16,219 wr)
==14411== D1  miss rate:       1.0% (      0.1%     +     6.2%  )
```

`option_two`
```
[tomasz.tomasz-laptop]> valgrind --tool=cachegrind ./loop-order-small
...
==14888== D   refs:      1,797,742  (1,534,743 rd   + 262,999 wr)
==14888== D1  misses:       20,960  (    1,588 rd   +  19,372 wr)
==14888== D1  miss rate:       1.2% (      0.1%     +     7.4%  )
```

*Confirmed!*

- I also suspect that doubling the size of the cache line will improve the cache utilization of `option_one` by a factor of 2.

Testing out the theory:
```
[tomasz.tomasz-laptop]> valgrind --tool=cachegrind --D1=32768,8,128 ./loop-order
...
==14995== D   refs:      112,061,743  (96,045,244 rd   + 16,016,499 wr)
==14995== D1  misses:        501,398  (     1,070 rd   +    500,328 wr)
==14995== D1  miss rate:         0.4% (       0.0%     +        3.1%  )
```

*Confirmed!*
