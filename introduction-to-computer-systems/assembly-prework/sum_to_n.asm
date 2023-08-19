section .text
global sum_to_n
sum_to_n:
    mov     rax, 0      ; return value
    mov     rsi, 1      ; loop counter
.testexpr:
    cmp     rsi, rdi
    jg      .return
.addtosum:
    add     rax, rsi
    inc     rsi
    jmp     .testexpr
.return:
    ret
