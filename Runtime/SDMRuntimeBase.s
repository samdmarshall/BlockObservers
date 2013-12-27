// Only the x86_64 function is fully explained; the other versions only comment the arch-specific notes

	.globl _SDMGenericGetSetInterceptor
	.globl _SDMGenericGetSetInterceptor_stret
#if __x86_64__
_SDMGenericGetSetInterceptor_stret:
	mov $1, %r10
	jmp L._SDMGenericGetSetInterceptor_do
_SDMGenericGetSetInterceptor:
	mov $0, %r10
L._SDMGenericGetSetInterceptor_do:
	push %rbp
	mov %rsp, %rbp // set up a stack frame for easier debugging
	
	// At this point we are correctly set up for returning the correct value in the correct place by definition.
	// ALL volatile state used for argument passing and returning must be preserved unconditionally before we call any functions.
	// We do not consider %rax a volatile state for this purpose, as it's not an input.
	// We also allow %r8 to be volatile since its value should not matter to a getter
	
	push %rdi // save arg0 (...) may be stret ptr or self
	push %rsi // save arg1 (...) may be self or _cmd
	push %rdx // save arg2 (...) may be _cmd or arg
	push %rcx // save arg3 (...) args
	push %r8  // save arg4 (...) args
	push %r9  // save arg5 (...) args
	sub $512, %rsp // reserve space on the stack to...
	fxsave (%rsp) // save off the full x87/MMX/SSE state

	// this function receives arg0 (self) and arg1 (_cmd) and MUST fire any notifications
	// AND return the IMP pointer for the original getter or setter
	cmp $1, %r10 // are we in stret mode?
	cmove %rsi, %rdi // if so, shift self up one
	cmove %rdx, %rsi // and also shift _cmd up one
	call _SDMFireGetterSetterNotificationsAndReturnIMP

	fxrstor (%rsp) // restore the full x87/MMX/SSE state
	add $512, %rsp // restore the stack pointer
    pop %r9  // restore arg5
    pop %r8  // restore arg4
    pop %rcx // restore arg3
	pop %rdx // restore arg2
	pop %rsi // restore arg1
	pop %rdi // restore arg0
	
 	// restore original frame pointer; after this instruction, this function disappears from the debugger's backtrace
 	pop %rbp

	// tail call to the original IMP - this right here is the part we just can't tell the compiler to do in C which makes the rest necessary
	jmp *%rax
#elif __i386__
_SDMGenericGetSetInterceptor_stret:
	mov $0x224, %edi
	jmp L._SDMGenericGetSetInterceptor_do
_SDMGenericGetSetInterceptor:
	mov $0x220, %edi
L._SDMGenericGetSetInterceptor_do:
	push %ebp
	mov %esp, %ebp // stack frame
	// i386 args go on the stack, non-fp registers are all volatile
	sub $520, %esp
	fxsave (%esp)
	
	// Stack:
	//  param3  - 544  param2  - 540  param1  - 536  retaddr - 532
	//  ebp     - 528  fxsave  - 16   _cmd    - 4    self    - 0
	sub $0x10, %esp // reserve args space on stack + alignment
	mov 0(%esp,%edi), %eax // set up self and _cmd arguments on the stack by copying from appropos spot in param area
	mov %eax, 0(%esp)
	mov 4(%esp,%edi), %eax
	mov %eax, 4(%esp)
	call _SDMFireGetterSetterNotificationsAndReturnIMP
	add $0x10, %esp // restore stack
	
	fxrstor (%esp)
	add $520, %esp
	
	pop %ebp
	jmp *%eax
#elif __armv7__ || __ARM_ARCH_7__ || __ARM_ARCH_7S__
	.syntax unified
	.code 32
	.align 5
_SDMGenericGetSetInterceptor_stret:
	mov r4, #1
	b L._SDMGenericGetSetInterceptor_do
_SDMGenericGetSetInterceptor:
	mov r4, #0
L._SDMGenericGetSetInterceptor_do:
	push {r7, lr}
	add r7, sp, #12 // establish stack frame
	
	push {r0-r3} // save all argument registers
	vstmdb sp!, {d0-d7} // save VFP regs
	cmp r4, #1 // if stret, shift self and _cmd up one
	itt eq
	moveq r0, r1
	moveq r1, r2
	blx _SDMFireGetterSetterNotificationsAndReturnIMP
	mov ip, r0 // save IMP in IP (intraprocedural scratch) register
	vldmia sp!, {d0-d7} // restore VFP regs
	pop {r0-r3} // restore argument registers
	
	pop {r7, lr} // destroy stack frame
	bx ip // tail call to IMP WITH interworking and WITHOUT linking!
#elif __arm64__
_SDMGenericGetSetInterceptor_stret:
	// Nothing to do here for arm64; a separate register is dedicated to stret
_SDMGenericGetSetInterceptor:
	stp fp, lr, [sp, #-16]! // save frame pointer and link register
	mov fp, sp // establish stack frame

	stp x0, x1, [sp, #-16]!
	stp x2, x3, [sp, #-16]!
	stp x4, x5, [sp, #-16]!
	stp x6, x7, [sp, #-16]!
	stp q0, q1, [sp, #-32]!
	stp q2, q3, [sp, #-32]!
	stp q4, q5, [sp, #-32]!
	stp q6, q7, [sp, #-32]!
	bl _SDMFireGetterNotificationsAndReturnImp
	mov x16, x0 // save IMP in ip0 (intraprocedural scratch)
	ldp q6, q7, [sp], #32
	ldp q4, q5, [sp], #32
	ldp q2, q3, [sp], #32
	ldp q0, q1, [sp], #32
	ldp x6, x7, [sp], #16
	ldp x4, x5, [sp], #16
	ldp x2, x3, [sp], #16
	ldp x0, x1, [sp], #16
	
	ldp fp, lr, [sp], #16
	b x16
#endif
