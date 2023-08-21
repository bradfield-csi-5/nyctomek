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
This version still needs to call `__builtin_ffsl` twice, which will probably use bit shifts to figure out the first set bit.  Looking at the sample input, that would be ~41 shifts in the average case for `memory_size` and 10 shifts in the average case for `page_size`.
```
uint64_t msizes[] = {1L << 32, 1L << 40, 1L << 52};
uint64_t psizes[] = {1L << 12, 1L << 16, 1L << 32};
```
Then there's a `sub` and another shift, so average case around ~41 + 10 + 1 + 1 = 53 instructions.  Given that `sub` and the shift instructions have a latency of about 1, we're looking at a total latency of 53 core clock cycles.  The latency of a single `div` of a 64-bit register on my architecture (Intel Skylake), is 35-88 core clock cycles.  So at best I would expect the optimized version to take 53 / 88 = ~60% as much time as the original version.

