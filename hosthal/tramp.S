
.text
.global tramptramp
tramptramp:
	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	/* FIXME vfp, xmm ...*/

	mov %rsp, %rdi
	call *80(%rsp)

	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax

	mov 8(%rsp), %rsp

	ret
