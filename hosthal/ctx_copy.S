
.text
.global ctx_Copy
ctx_Copy:
	mov %rbx, 0*8(%rdi)
	mov %r12, 1*8(%rdi)
	mov %r13, 2*8(%rdi)
	mov %r14, 3*8(%rdi)
	mov %r15, 4*8(%rdi)
	mov %rsp, 5*8(%rdi)
	mov %rbp, 6*8(%rdi)

	ret
