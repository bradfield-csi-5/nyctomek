section .text
global binary_convert
binary_convert:
    mov		rax, 0
.loopexpr:
    movzx	rsi, byte [rdi]
    cmp		rsi, 0
    je 		.return
    shl     rax, 1
    add		rax, rsi
    sub 	rax, 48
    add		rdi, 1
    jmp 	.loopexpr
.return:
    ret
