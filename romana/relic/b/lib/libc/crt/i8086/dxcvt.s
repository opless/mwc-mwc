////////
/
/ Intel 8086 floating point.
/ integer types to double.
/ small model.
/
////////

	.globl	dicvt
	.globl	ducvt
	.globl	dlcvt
	.globl	dvcvt
	.globl	_fpac_

////////
/
/ ** dicvt -- convert integer to double float.
/
/ this routine is called directly by the compiler to convert an integer
/ into a double precision floating point number. no checking is done if
/ the integer overflows when it is negated.
/
/ compiler calling sequence:
/	push	<int argument>
/	call	dicvt
/	add	sp,2
/
/ outputs:
/	_fpac_=result.
/
/ uses:
/	ax, bx, cx, dx
/
////////

i	=	8			/ int argument.

dicvt:	push	si			/ standard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ linkage.

	sub	dx,dx			/ top half = 0.
	subb	bh,bh			/ bh=0 (positive)
	mov	ax,i(bp)		/ get argument.
	or	ax,ax			/ check its sign,
	jns	l0			/ jump if positive.
	neg	ax			/ negate.
	movb	bh,$0x80		/ set sign flag.
	jmp	l0			/ go to common code.

////////
/
/ ** ducvt -- convert unsigned integer to double float.
/
/ this routine is called directly by the compiler to convert an unsigned
/ integer to double precision floating point. no overflows are possible.
/
/ compiler calling sequence:
/	push	<unsigned argument>
/	call	ducvt
/	add	sp,2
/
/ outputs:
/	_fpac_=result.
/
/ uses:
/	ax, bx, cx, dx
/
////////

u	=	8			/ unsigned int argument.

ducvt:	push	si			/ standard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ linkage.

	sub	dx,dx			/ top half = 0.
	subb	bh,bh			/ bh=0 (positive)
	mov	ax,u(bp)		/ low half = argument.
	jmp	l0			/ go to common code.

////////
/
/ ** dlcvt -- convert long integer to double float.
/ 
/ this routine is called directly by the compiler to convert a long (32
/ bit) integer to double precision floating point. no checking is  done
/ for overflow when the long integer is negated.
/
/ compiler calling sequence:
/	push	<long argument>
/	call	dlcvt
/	add	sp,4
/
/ outputs:
/	_fpac_=result.
/
/ uses:
/	ax, bx, cx, dx
/
////////

l	=	8			/ long argument.

dlcvt:	push	si			/ stadard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ linkage.

	subb	bh,bh			/ sign = positive.
	mov	dx,l+2(bp)		/ fetch.
	mov	ax,l+0(bp)		/ long.
	or	dx,dx			/ test the sign.
	jns	l0			/ positive, go to common code.
	neg	dx			/ negate
	neg	ax			/ the
	sbb	dx,$0			/ long int.
	movb	bh,$0x80		/ set the sign flag.
	jmp	l0			/ go to common code.

////////
/
/ ** dvcvt -- convert unsigned long integer to double float
/
/ this routine is called directly by the compiler to convert a long (32
/ bit) unsigned integer to double precision floating point. no overflow
/ is possible.
/
/ compiler calling sequence:
/	push	<unsigned long argument>
/	call	dvcvt
/	add	sp,4
/
/ outputs:
/	_fpac_=result.
/
/ uses:
/	ax, bx, cx, dx
/
////////

v	=	8			/ unsigned long argument.

dvcvt:	push	si			/ standard
	push	di			/ c
	push	bp			/ function
	mov	bp,sp			/ linkage.

	subb	bh,bh			/ sign = positive.
	mov	dx,v+2(bp)		/ fetch
	mov	ax,v+0(bp)		/ unsigned long.

l0:	sub	cx,cx			/ exp=0 (for 0)
	mov	si,dx			/ test for
	or	si,ax			/ zeroness.
	jz	l3			/ jump if zero, exp=0, frac=0.

	movb	cl,$128+32		/ cx=binary point to right of ax.

l1:	shl	ax,$1			/ shift up
	rcl	dx,$1			/ until
	jc	l2			/ the hidden bit
	decb	cl			/ just
	jmp	l1			/ appears.

l2:	shrb	cl,$1			/ make room
	rcr	dx,$1			/ for the
	rcr	ax,$1			/ sign bit.
	orb	cl,bh			/ zap in sign.

l3:	movb	_fpac_+7,cl		/ save result
	mov	_fpac_+5,dx		/ into			(odd)
	mov	_fpac_+3,ax		/ the			(odd)
	sub	ax,ax			/ function
	movb	_fpac_+2,al		/ return
	mov	_fpac_+0,ax		/ location

	pop	bp			/ standard
	pop	di			/ c
	pop	si			/ function
	ret				/ return.
