# Practice Problem 3.32
```
_______________________________________________________________________________________________________________

        Instruction                             State values (at beginning)
_______________________________     _____________________________________________________

Label   PC          Instruction     %rdi    %rsi    %rax            %rsp        *%rsp       Description
_______________________________________________________________________________________________________________
M1      0x400560    callq            10      ---     ---       0x7fffffffe820    ---        Call first(10)
F1      0x400548    lea              10      ---     ---       0x7fffffffe818    0x400565   Entry of first
F2      0x40054c    sub              10      11      ---       0x7fffffffe818    0x400565   Prepare to call last
F3      0x400550    callq             9      11      ---       0x7fffffffe818    0x400565   Call last(9, 11)
L1      0x400540    mov               9      11      ---       0x7fffffffe810    0x400555   Entry of last
L2      0x400543    imul              9      11       9        0x7fffffffe810    0x400555   u*v
L3      0x400547    retq              9      11      99        0x7fffffffe810    0x400555   Return 99 from last
F4      0x400555    repz retq         9      11      99        0x7fffffffe818    0x400565   Return 99 from first
M2      0x400565    mov               9      11      99        0x7fffffffe820    ---        Resume main
```
