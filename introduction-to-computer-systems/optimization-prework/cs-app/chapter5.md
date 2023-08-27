## Practice Problem 5.3

```
Code    min     max     incr    square
A.       1      91       90       90
B.      91       1       90       90
C.       1       1       90       90
```

## Practice Problem 5.5

A.
For degree `n`, `2n` additions are performed: one floating point addition to update `result` and one integer addition to increment `i`, and `2n` floating point multiplications are preformed: one to update `result` and one to update `xpwr`.

B.
Lines 7-8:

```
result += a[i] * xpwr;
xpwr = x * xpwr;
```

 The data dependency between iterations is that `xpwr` needs to be updated via `xpwr = x * xpwr` before the `result += a[i] * xpwr` can be executed in the next iteration.  That introduces a latency of 5 clock cycles.  Although there are two floating point multiplications in each iteration, they can probably be parallelized because of the floating point throughput of 0.5.  As long at store step of `xpwr = x * xpwr` isn't completed before the `a[i] * xpwr` multiplication in the same iteration executes, there shouldn't be a data dependency.  Furthermore, it seems like the addition step of `result += a[i] * xpwr` can happen in parallel with the `a[i] * xpwr` step of the **next** iteration, so there's not data dependency between iterations for those two operations.

 ## Practice Problem 5.6

 A.
 For degree `n`, the code performs `n` floating point additions and `n` floating point multiplication operation.

 B.
 Line 7:
 ```
 result = a[i] + x*result;
 ```
 The floating point addition contributes a latency of 5 clock cycles while the floating point addition contributes a latency of 3 clock cycles.  Since the code has a measured CPE of 8.00, that means the two operations weren't parallelized.  In other words, there's a data dependency between consecutive iterations on both the addition and multiplication operation.  Within a single iteration, the multiplication needs to complete before the addition can execute, and until the addition completes, the multiplication for the next iteration can't begin, because it depends on the sum of the addition being written back to `result`.

C.
The function from Practice Problem 5.5 can run faster because it can be parallelized as described in the solution to Practice Problem 5.5 and therefore take advantage of the floating point multiplication throughput of 0.5.

## Practice Problem 5.7
```
/* 5 x 1 loop unrolling */
void combine5(vec_ptr v, data_t *dest)
{
    const long unrollFactor = 5;

    long i;
    long length = vec_length(v);
    long limit = length - (unrollFactor - 1);
    data_t *data = get_vec_start(v);
    data_t acc = IDENT;

    /* Combine 5 elements at a time. */
    for (i = 0; i < limit; i += unrollFactor) {
        acc = acc OP data[i];
        acc = acc OP data[i+1];
        acc = acc OP data[i+2];
        acc = acc OP data[i+3];
        acc = acc OP data[i+4];
    }
    *dest = acc;
}
```
## Practice Problem 5.8
```
r = ((r * x) * y) * z; /* A1 */
r = (r * (x * y)) * z; /* A2 */
r = r * ((x * y) * z); /* A3 */
r = r * (x * (y * z)); /* A4 */
r = (r * x) * (y * z); /* A5 */
```
- A1 will require all 3 `mul` operations to complete before the iteration can proceed.  With multiplication having a latency of 5 clock cycles, and the loop being unrolled 3x, that gives us a CPE floor of 5 * 3 / 3 = 5.
- A2 will allow the inner `(x * y)` to be performed independently, leaving two sequential `mul` operations in each loop, for a CPE floor of 5 * 2 / 3 = 3.33.
- A3 will allow all 3 `mul` operations to be parallelized, for a CPE floor of 5 * 1 / 3 = 1.67.
- A4 will allow all 3 `mul` operations to be parallelized, for a CPE floor of 5 * 1 / 3 = 1.67.
- A5 will allow the `(y * z)` operation to be parallelized, for a CPE floor of 5 * 2 / 3 = 3.33.

## Practice Problem 5.10

A.
`copy_array(a+1,a,999)` would copy all elements from `a[i]` to `a[i-1]` for `0 < i < 1000`.  In other words, starting with the second element, it would copy every element back one place.

B.
`copy_array(a,a+1,999)` would set each element to the value of the first element in the array.

C.
B is slower than A because it has a critical path in which the `load` operation in `dest[i] = src[i]` has to wait for the `store` operation from the previous iteration to update the store buffer in the store functional unit.  Therefore, the code in B can't benefit from out-of-order execution.

D.
I would expect `copy_array(a,a,999)` to have performance similar to `copy_array(a+1,a,999)`.  In other words, I would expect a CPE of around 1.2 for the unrolled code.  In `copy_array(a,a,999)`, there's no read/write dependency.  The `load` operation in each iteration does not depend on a previous `store`.

## Practice Problem 5.11

The CPE is so high because there's a read/write dependency from one iteration to the next.  The the `vmovss -4(%rsi,%rax,4), %xmm0` instruction that reads `p[i-1]` at each iteration can't proceed until the `vmovss %xmm0, (%rsi,%rax,4)` instruction that stores `p[i]` in the previous iteration updates the store buffer in the store functional unit.  And that also means that it can't proceed until the previous iteration's load and floating point addition have completed, because the store at depends on the outcome of those operations.  The code can't benefit from out-of-order execution.

## Practice Problem 5.12

```
void psum1(float a[], float p[], long n)
{
    long i;
    float acc = 0;
    for(i = 0; i < n; i++) {
        acc += a[i];
        psum[i] = acc;
    }
}
```
