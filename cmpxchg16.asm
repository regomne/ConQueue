
;.XMM

public CompareAndExchange16

.code
;extern "C" bool CompareAndExchange16(volatile void* toXch, volatile void* ptr11,
;    usize_t cnt1, volatile void* ptr12, usize_t cnt2);
; 10 18 20 28 30

CompareAndExchange16 proc
    push rbx;
    mov rbx, r9;
    mov r9, rcx;
    mov rcx, [rsp+30h];
    mov rax, rdx;
    mov rdx, r8;
    lock cmpxchg16b[r9];
    sete al;
    pop rbx;
	ret
CompareAndExchange16 endp
end