/ (lgl-
/ 	COHERENT Driver Kit Version 1.1.0
/ 	Copyright (c) 1982, 1990 by Mark Williams Company.
/ 	All rights reserved. May not be copied without permission.
/ -lgl)
////////
/
/ Tiac Network Assembler Support
/
/	tngetc( np )		-- get a character from a tiac network buffer
/	tnputc( np, c)		-- put a character into a tiac network buffer
/	tucopy( np, up, n)	-- copy n bytes from tiac buffer to user space
/	utcopy( up, np, n)	-- copy n bytes from user space to tiac buffer
/
////////

	.globl	tngetc_
	.globl	tnputc_
	.globl	tucopy_
	.globl	utcopy_

////////
/
/ Tngetc ( np )
/
/	Input:	np = pointer to seg:offset pair for tiac network buffer
/
/	Action:	Read character from network buffer, increment offset.
/
/	Return:	Character.
/
////////

tngetc_:				/ tngetc( np )
	push	si			/ char **np;
	push	bp			/ {
	mov	bp, sp			/	register char c;	/* AX */
	mov	bx, 6(bp)		/	register char *cp;	/* SI */
	push	ds			/
	lds	si, (bx)		/	cp = *np;
	cld				/
	lodsb				/	c = *cp++;
	pop	ds			/
	mov	(bx), si		/	*np = cp;
	subb	ah, ah			/
	pop	bp			/	return( c );
	pop	si			/ }
	ret

////////
/
/ Tnputc ( np, c )
/ char **np;
/ char c;
/
/	Input:	np = pointer to seg:offset pair for tiac network buffer
/		c  = character to transfer
/
/	Action:	Transfer character C to network buffer, increment offset.
/
/	Return:	Character C.
/
////////

tnputc_:				/ tnputc( np, c )
	push	di			/ char **np;			/* BX */
	push	bp			/ char c;			/* AX */
	mov	bp, sp			/ {
	mov	ax, 8(bp)		/	register char *cp;	/* DI */
	mov	bx, 6(bp)		/
	push	es			/
	les	di, (bx)		/	cp = *np;
	cld				/
	stosb				/	*cp++ = c;
	pop	es			/
	mov	(bx), di		/	*np = cp;
	pop	bp			/
	pop	di			/	return c;
	ret				/ }

////////
/
/ utcopy( up, np, n )
/ char * up;
/ char **np;
/ unsigned n;
/
/	Input:	up = offset in user data space for source data
/		np = pointer to seg:offset pair	for tiac network buffer
/		n  = number of bytes to transfer
/
/	Action:	Copy N bytes from user data space to network data space.
/		Add N to network data space offset.
/
/	Return: None.
/
////////

utcopy_:				/ utcopy( up, np, n )
	push	si			/
	push	di			/ register char *  up;		/* SI */
	push	bp			/ register char ** np;		/* BX */
	mov	bp, sp			/ register unsigned n;		/* CX */
	push	ds			/
	push	es			/ {
	mov	bx, 10(bp)		/	register char * cp;	/* DI */
					/
	les	di, (bx)		/	cp = *np;
					/
	mov	si, 8(bp)		/	up;
	mov	ds, uds_		/
					/
	mov	cx, 12(bp)		/	n;
					/
	cld				/
	clc				/
	rcr	cx, $1			/
	rep				/	for ( ; n != 0; --n )
	movsw				/		*cp++ = *up++;
	rcl	cx, $1			/
	rep				/
	movsb				/
					/
	pop	es			/
	pop	ds			/
	mov	(bx), di		/	*np = cp;
	pop	bp			/ }
	pop	di
	pop	si
	ret

////////
/
/ tucopy( np, up, n )
/ char **np;
/ char * up;
/ unsigned n;
/
/	Input:	np = pointer to seg:offset pair	for tiac network buffer
/		up = offset in user data space for destination
/		n  = number of bytes to transfer
/
/	Action:	Copy N bytes from network data space to user data space.
/		Add N to network data space offset.
/
/	Return: None.
/
////////

tucopy_:				/ tucopy( np, up, n )
	push	si			/
	push	di			/ register char ** np;		/* BX */
	push	bp			/ register char *  up;		/* DI */
	mov	bp, sp			/ register unsigned n;		/* CX */
	push	ds			/
	push	es			/ {
	mov	bx, 8(bp)		/	register char * cp;	/* SI */
					/
	mov	di, 10(bp)		/	up;
	mov	es, uds_		/
					/
	lds	si, (bx)		/	cp = *np;
					/
	mov	cx, 12(bp)		/	n;
					/
	cld				/
	clc				/
	rcr	cx, $1			/
	rep				/	for ( ; n != 0; --n )
	movsw				/		*up++ = *cp++;
	rcl	cx, $1			/
	rep				/
	movsb				/
					/
	pop	es			/
	pop	ds			/
	mov	(bx), si		/	*np = cp;
	pop	bp			/ }
	pop	di
	pop	si
	ret
