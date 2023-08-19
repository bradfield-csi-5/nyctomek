section .text
global pangram
pangram:
    mov     rax, 0
.loopOverInput:
    movzx   rsi, byte [rdi]
    inc     rdi
    cmp     rsi, 0
    je      .computeRval
    cmp     rsi, 65         ; rsi < 'A'
    jl      .loopOverInput
    cmp     rsi, 122        ; rsi > 'z'
    jg      .loopOverInput
    cmp     rsi, 91
    jl      .recordCharacter
    cmp     rsi, 96
    jg      .recordCharacter
    jmp     .loopOverInput
.recordCharacter:
    sub     rsi, 65
    cmp     rsi, 25
    jg      .biasForLowercase
    jmp     .record
.biasForLowercase:
    sub     rsi, 32
.record:
    mov     rcx, rsi
    mov     rbx, 1
    shl     rbx, cl
    or      rax, rbx
    jmp .loopOverInput
.computeRval:
    cmp     rax, 0x03FFFFFF
    je      .setRvalToTrue
    mov     rax, 0
    jmp     .return
.setRvalToTrue:
    mov     rax, 1
.return:
	ret
