# Practice Problem 5.3

```
Code    min     max     incr    square
A.       1      91       90       90
B.      91       1       90       90
C.       1       1       90       90
```

# Practice Problem 5.5

A.
For degree `n`, `2n` additions are performed: one floating point addition to update `result` and one integer addition to increment `i`, and `2n` floating point multiplications are preformed: one to update `result` and one to update `xpwr`.

B.
Lines 7-8:

```
result += a[i] * xpwr;
xpwr = x * xpwr;
```

 The data dependency between iterations is that `xpwr` needs to be updated via `xpwr = x * xpwr` before the `result += a[i] * xpwr` can be executed in the next iteration.  That introduces a latency of 5 clock cycles.  Although there are two floating point multiplications in each iteration, they can probably be parallelized because of the floating point throughput of 0.5.  As long at store step of `xpwr = x * xpwr` isn't completed before the `a[i] * xpwr` multiplication in the same iteration executes, there shouldn't be a data dependency.  Furthermore, it seems like the addition step of `result += a[i] * xpwr` can happen in parallel with the `a[i] * xpwr` step of the **next** iteration, so there's not data dependency between iterations for those two operations.

 # Practice Problem 5.6

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

