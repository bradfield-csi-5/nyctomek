# pagecount

### Which instructions would you expect your compiler to generate for this function?

I would expect the compiler to generate `div`/`idiv` instructions.

### What does it in fact generate?

With optimization disabled, it generates a `divq` instruction.
```
; gcc -O0 -g -o pagecount pagecount.c

uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
    1189:       f3 0f 1e fa             endbr64 
    118d:       55                      push   %rbp
    118e:       48 89 e5                mov    %rsp,%rbp
    1191:       48 89 7d f8             mov    %rdi,-0x8(%rbp)
    1195:       48 89 75 f0             mov    %rsi,-0x10(%rbp)
  return memory_size / page_size;
    1199:       48 8b 45 f8             mov    -0x8(%rbp),%rax
    119d:       ba 00 00 00 00          mov    $0x0,%edx
    11a2:       48 f7 75 f0             divq   -0x10(%rbp)
}
    11a6:       5d                      pop    %rbp
    11a7:       c3                      ret
```
Performance at `O0`:
```
pagecount(): 0.93s to run 100000000 tests (9.34ns per test)
```

### If you change the optimization level, is the function substantially different?
Not really.  The only real difference as the optimization level changes is that there is less manipulation of the stack, and the `divq` is switched out for a `div.`

Optimizing at `Og`:
```
; gcc -Og -g -o pagecount pagecount.c
uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
    1189:       f3 0f 1e fa             endbr64 
    118d:       48 89 f8                mov    %rdi,%rax
  return memory_size / page_size;
    1190:       ba 00 00 00 00          mov    $0x0,%edx
    1195:       48 f7 f6                div    %rsi
}
    1198:       c3                      ret    
```
Performance at `Og`:
```
pagecount(): 0.82s to run 100000000 tests (8.21ns per test)
```
Optimizing at `O1`:
```
; gcc -O1 -g -o pagecount pagecount.c
uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
    1189:       f3 0f 1e fa             endbr64 
    118d:       48 89 f8                mov    %rdi,%rax
  return memory_size / page_size;
    1190:       ba 00 00 00 00          mov    $0x0,%edx
    1195:       48 f7 f6                div    %rsi
}
    1198:       c3                      ret
```
Performance at `O1`:
```
pagecount(): 0.72s to run 100000000 tests (7.18ns per test)
```
Optimizing at `O2`:
```
; gcc -O2 -g -o pagecount pagecount.c
uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
    1320:       f3 0f 1e fa             endbr64 
  return memory_size / page_size;
    1324:       48 89 f8                mov    %rdi,%rax
    1327:       31 d2                   xor    %edx,%edx
    1329:       48 f7 f6                div    %rsi
}
    132c:       c3                      ret
```
Performance at `O2`:
```
pagecount(): 0.75s to run 100000000 tests (7.46ns per test)
```
Optimizing at `O3`:
```
; gcc -O3 -g -o pagecount pagecount.c
uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
    1300:       f3 0f 1e fa             endbr64 
  return memory_size / page_size;
    1304:       48 89 f8                mov    %rdi,%rax
    1307:       31 d2                   xor    %edx,%edx
    1309:       48 f7 f6                div    %rsi
}
    130c:       c3                      ret    
```
Performance at `O3`:
```
pagecount(): 0.77s to run 100000000 tests (7.65ns per test)
```
### Use godbolt.org to explore a few different compilers.  Do any of them generate substantially different instructions?
Again, not really, `clang` at `O2` emits a separate branch for the case where `memory_size` is small enough to fit into 32-bits.  It generates assembly to use `divl` instead of `divq` for that case:
```
pagecount:                              # @pagecount
        movq    %rdi, %rax
        movq    %rdi, %rcx
        orq     %rsi, %rcx
        shrq    $32, %rcx
        je      .LBB0_1
        xorl    %edx, %edx
        divq    %rsi
        retq
.LBB0_1:
        xorl    %edx, %edx
        divl    %esi
        retq
```
### By using Agner Fog’s instruction tables or reviewing CS:APP chapter 5.7, can you determine which of the generated instructions may be slow?
Definitely the `divq`/`div`.

### Noting that a page size is always a power of 2, and that the size of memory will always be cleanly divisible by the page size, can you think of a performance optimization we could employ? You are welcome to change the function signature and test runner code.
We can use bit shifts instead of division.  The exercise files already shipped with a version of `pagecount` that uses bit shift instead of division.
```
uint64_t pagecount(uint64_t memory_size, uint64_t page_size) {
  uint64_t a = __builtin_ffsl(memory_size);
  uint64_t b = __builtin_ffsl(page_size);
  return 1LLU << (a - b);
}
```
### How much of an improvement would you expect to see?

```
gcc -O0 -o pagecount pagecount.c && objdump -d pagecount
...
...
0000000000001189 <pagecount>:
    1189:       f3 0f 1e fa             endbr64                             ; n/a
    118d:       55                      push   %rbp                         ; 3
    118e:       48 89 e5                mov    %rsp,%rbp                    ; 1
    1191:       48 89 7d e8             mov    %rdi,-0x18(%rbp)             ; 1
    1195:       48 89 75 e0             mov    %rsi,-0x20(%rbp)             ; 1
    1199:       48 8b 45 e8             mov    -0x18(%rbp),%rax             ; 1
    119d:       48 c7 c2 ff ff ff ff    mov    $0xffffffffffffffff,%rdx     ; 1
    11a4:       48 0f bc c0             bsf    %rax,%rax                    ; 3
    11a8:       48 0f 44 c2             cmove  %rdx,%rax                    ; 1
    11ac:       48 83 c0 01             add    $0x1,%rax                    ; 1
    11b0:       48 98                   cltq                                ; n/a
    11b2:       48 89 45 f0             mov    %rax,-0x10(%rbp)             ; 1
    11b6:       48 8b 45 e0             mov    -0x20(%rbp),%rax             ; 1
    11ba:       48 c7 c2 ff ff ff ff    mov    $0xffffffffffffffff,%rdx     ; 1
    11c1:       48 0f bc c0             bsf    %rax,%rax                    ; 3
    11c5:       48 0f 44 c2             cmove  %rdx,%rax                    ; 1
    11c9:       48 83 c0 01             add    $0x1,%rax                    ; 1
    11cd:       48 98                   cltq                                ; n/a
    11cf:       48 89 45 f8             mov    %rax,-0x8(%rbp)              ; 1
    11d3:       48 8b 45 f0             mov    -0x10(%rbp),%rax             ; 1
    11d7:       89 c2                   mov    %eax,%edx                    ; 1
    11d9:       48 8b 45 f8             mov    -0x8(%rbp),%rax              ; 1
    11dd:       89 c1                   mov    %eax,%ecx                    ; 1
    11df:       89 d0                   mov    %edx,%eax                    ; 1
    11e1:       29 c8                   sub    %ecx,%eax                    ; 1
    11e3:       ba 01 00 00 00          mov    $0x1,%edx                    ; 1
    11e8:       89 c1                   mov    %eax,%ecx                    ; 1
    11ea:       48 d3 e2                shl    %cl,%rdx                     ; 2
    11ed:       48 89 d0                mov    %rdx,%rax                    ; 1
    11f0:       5d                      pop    %rbp                         ; 2
    11f1:       c3                      ret                                 ; n/a
```
There are 31 instructions in this version of `pagecount`.  I've commented the disassembly with the latency for each instruction based on Abner Fog's instruction tables for my architecture, Skylake.  The function should take around 35 cycles.

The `O0` version using `div` only has 10 instructions, but `div` has a latency of 35-88 cycles.  I'll assume it averages out to the median, of 62 cycles, for a total of about 70 cycles per function run.  So I would expect the version using `__buildin_ffsl` to take about `35 / 70 = 50%` as much time.

### Go ahead and make the improvement, and measure the speed up. Did this match your expectations?

```
pagecount(): 0.42s to run 100000000 tests (4.18ns per test)
```
Comparing to the `O0` `div`-based version's performance (9.34ns ns per test), the `O0` `shl`-based version ran in `4.18 / 9.34 = 45%` of the time.  That certainly lines up with expectations.

### Consider, what is stopping the compiler from making the same optimization that you did?

The compiler has no way of knowing that `page_size` will be a power of 2, so it can't assume that it can use `shl` in place of `div`.

### Stretch goals

Using a profiling tool like perf on Linux or Instruments on macOS, see if you can measure the number of CPU cycles used for each version. Does this correspond to our measure of CPU time?

`O0` `div`-based:
```
 perf stat ./pagecount
pagecount(): 0.91s to run 100000000 tests (9.05ns per test)

 Performance counter stats for './pagecount':

          1,705.70 msec task-clock                       #    0.999 CPUs utilized             
                23      context-switches                 #   13.484 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
                63      page-faults                      #   36.935 /sec                      
     5,118,606,462      cycles                           #    3.001 GHz                       
     9,703,582,944      instructions                     #    1.90  insn per cycle            
       400,606,681      branches                         #  234.863 M/sec                     
            20,261      branch-misses                    #    0.01% of all branches           

       1.707218652 seconds time elapsed

       1.706246000 seconds user
       0.000000000 seconds sys
```
`00` `shl`-based:
```
$ perf stat ./pagecount
pagecount(): 0.47s to run 100000000 tests (4.71ns per test)

 Performance counter stats for './pagecount':

          1,244.34 msec task-clock                       #    1.000 CPUs utilized             
                 6      context-switches                 #    4.822 /sec                      
                 0      cpu-migrations                   #    0.000 /sec                      
                63      page-faults                      #   50.629 /sec                      
     3,774,321,155      cycles                           #    3.033 GHz                       
    11,802,890,892      instructions                     #    3.13  insn per cycle            
       400,492,089      branches                         #  321.850 M/sec                     
            15,391      branch-misses                    #    0.00% of all branches           

       1.244962392 seconds time elapsed

       1.244973000 seconds user
       0.000000000 seconds sys
```
The `shl`-based code uses `3,774,321,155 / 5,118,606,462 = ~73%` as many CPU cycles as the `div`-based version.  That's more than the 45% measured by the test runner code.  But that's because the CPU cycles reported are for the entire program, including outer loop that calls `pagecount`, so they include some outside noise.

