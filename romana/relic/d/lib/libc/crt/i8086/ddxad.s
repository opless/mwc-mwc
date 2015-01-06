////////
/
/ Intel 8086 C runtime.
/ assigned add to a double float.
/ small model.
/
////////

	.globl	ddladd
	.globl	ddradd
	.globl	_fpac_
	.globl	dladd

////////
/
/ ** ddladd -- assigned double add (lvalue)
/ ** ddradd -- assigned double add (rvalue)
/
/ these two routines are called directly by the compiler to do double
/ floating assigned addition. these small routines make the  compiler
/ simpler, and get rid of a very bulky code sequence that would  have
/ to be generated for a fairly uncommon operation.
/
/ compiler calling sequences:
/	push	<right double rvalue>
/	lea	ax,<left double lvalue>
/	push	ax
/	call	ddradd
/	add	sp,10
/
/	lea	ax,<right double lvalue>
/	push	ax
/	lea	ax,<left double lvalue>
/	push	ax
/	call	ddladd
/	add	sp,4
/
/ outputs:
/	_fpac_=result.
/
/ uses:
/	ax, bx, cx, dx
/
////////

pa	=	8			/ pointer to left
pb	=	10			/ pointer to right
b	=	10			/ right

ddladd:	push	si			/ standard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ entry

	mov	ax,pb(bp)		/ ax=pointer to right operand
	jmp	l0			/ finish in common code

ddradd:	push	si			/ standard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ entry

	lea	ax,b(bp)		/ ax=pointer to right operand

l0:	push	ax			/ second arg is right lvalue.
	mov	si,pa(bp)		/ si=pointer to left.
	push	6(si)			/ push
	push	4(si)			/ the
	push	2(si)			/ left
	push	0(si)			/ rvalue
	call	dladd			/ do the addition.
	add	sp,$10			/ pop args

	mov	ax,_fpac_+6		/ copy
	mov	6(si),ax		/ the
	mov	ax,_fpac_+4		/ result
	mov	4(si),ax		/ back
	mov	ax,_fpac_+2		/ to
	mov	2(si),ax		/ the
	mov	ax,_fpac_+0		/ left
	mov	0(si),ax		/ operand.

	pop	bp			/ standard
	pop	di			/ c
	pop	si			/ function
	ret				/ return.
