/ (lgl-
/ 	COHERENT Driver Kit Version 1.0.0
/ 	Copyright (c) 1982, 1990 by Mark Williams Company.
/ 	All rights reserved. May not be copied without permission.
/ -lgl)
	.globl	grread_
	.globl	grwrite_
	.globl	mmgo_

////////
/
/ State driven code
/
/	Input:	DS:SI - input string
/		ES:DI - current screen location
/		SS:BP - terminal attributes
/		CX    - input count
/		BP    - references terminal information
/		AL    - character
/		BH    - (usually) kept zeroed for efficiency
/		DH    - current row
/		DL    - current column
/
////////

#ifdef	HERCULES
	VSEG	= 0xB000	/ Video Display Segment
	BANKSZ	= 8192		/ Size of Display Banks
	LROW	= 24		/ Last legal row
	NCR	= 8		/ number of horizontal lines per char
	NCR2	= 4		/ number of horizontal lines per char / 2
	NCR4	= 2		/ number of horizontal lines per char / 4
	NHB	= 90		/ number of horizontal bytes per line
#else
#ifdef TECMAR
	VSEG	= 0xA000	/ Video Display Segment
	BANKSZ	= 32768		/ Size of Display Banks
	LROW	= 24		/ Last legal row
	NCR	= 16		/ number of horizontal lines per char
	NCR2	= 8		/ number of horizontal lines per char / 2
	NCR4	= 4		/ number of horizontal lines per char / 4
	NHB	= 80		/ number of horizontal bytes per line
#else
	VSEG    = 0xB800	/ Video Display Segment
	BANKSZ	= 8192		/ Size of Display Banks
	LROW	= 24		/ Last legal row
	NCR	= 8		/ number of horizontal lines per char
	NCR2	= 4		/ number of horizontal lines per char / 2
	NCR4	= 2		/ number of horizontal lines per char / 4
	NHB	= 80		/ number of horizontal bytes per line
#endif
#endif

	NRB	= NHB*NCR	/ number of bytes per character row
	NRB2	= NHB*NCR2	/ number of bytes per character row / 2
	NRB4	= NHB*NCR4	/ number of bytes per character row / 4


	ZERO	= bh		/ (almost) always zero
	ROW	= dh		/ currently active vertical position
	COL	= dl		/ currently active horizontal position
	POS	= di		/ currently active display address

	CSR	= 0x3D9		/ Color Select Register
	MSR	= 0x3D8		/ Mode Select Register
	XMSR	= 0x3DA		/ Extended Mode Select Register


////////
/
/ Magic constants from <sys/io.h>
/
////////

	IO_SEG	= 0
	IO_IOC	= 2
	IO_SEEK	= 4
	IO_BASE	= 8

	IOSYS	= 0

////////
/
/ Data
/
////////

MM_FUNC		= 0		/ current state
MM_PORT		= 2		/ adapter base i/o port
MM_BASE		= 4		/ adapter base memory address
MM_ROW		= 6		/ screen row
MM_COL		= 7		/ screen column
MM_POS		= 8		/ screen position
MM_ATTR		= 10		/ attributes
MM_N1		= 11		/ numeric argument 1
MM_N2		= 12		/ numeric argument 2
MM_BROW		= 13		/ base row
MM_EROW		= 14		/ end row
MM_LROW		= 15		/ legal row limit
MM_SROW		= 16		/ saved cursor row
MM_SCOL		= 17		/ saved cursor column
MM_IBROW	= 18		/ initial base row
MM_IEROW	= 19		/ initial end row
MM_FLIP		= 20
MM_MASK		= 22
MM_ULINE	= 24
MM_CURSE	= 26
MM_VIS		= 28		/ set to -1 to make cursor visible
MM_NCOL		= 30
MM_WRAP		= 31

/ ASCII characters
AZERO		= 0x30
CLOWER		= 0x63
HLOWER		= 0x68
LLOWER		= 0x6C
SEMIC		= 0x3B

	.prvd
mmdata:	.word	mminit, 0x03D4, VSEG
	.byte	0, 0
	.word	0
	.byte	0x7, 0, 0, 0, LROW-1, LROW, 0, 0, 0, LROW-1
	.word	0, -1, 0, 0xff
	.word	0
	.byte	80
	.byte	1

	.shri
#ifdef	HERCULES
crtdata:.byte	 54,  45,  45,   8,  91,   1,  86,  88
	.byte	  2,   3,  32,   0,   0,   0,   0,   0
#else
#ifdef TECMAR
crtdata:.byte	 56,  40,  43,   8, 127,   6, 100, 112
	.byte	  3,   1,  32,   0,   0,   0,   0,   0
#else
crtdata:.byte	 56,  40,  43,   8, 127,   6, 100, 112
	.byte	  2,   1,  32,   0,   0,   0,   0,   0
#endif
#endif
	.shri

////////
/
/ mmgo( iop )		- Entry point for text stream output
/ IO *iop;
/
////////

mmgo_:
	push	si
	push	di
	push	bp
	mov	bp, sp
	push	ds
	push	es
	cld
	mov	bx, 8(bp)		/ iop
	mov	si, IO_BASE(bx)		/ iop->io_base
	mov	cx, IO_IOC(bx)		/ iop->io_ioc
	cmp	IO_SEG(bx), $IOSYS
	je	0f
	mov	bx, uds_
	mov	ds, bx
0:	mov	bp, $mmdata
	movb	ROW, MM_ROW(bp)
	movb	COL, MM_COL(bp)
	mov	es,  MM_BASE(bp)
	mov	POS, MM_POS(bp)
	mov	ax, MM_CURSE(bp)
	and	ax, MM_VIS(bp)
#ifdef TECMAR
	xor	es:[NHB*6](POS), ax
	xor	es:[NHB*6]+BANKSZ(POS), ax
	xor	es:[NHB*7](POS), ax
	xor	es:[NHB*7]+BANKSZ(POS), ax
#else
	xor	es:[NHB*3](POS), ax	/ turn cursor off
	xor	es:[NHB*3]+BANKSZ(POS), ax
#endif
	sub	bx, bx
	ijmp	MM_FUNC(bp)

exit:	movb	bl, ROW			/ reposition to ROW and COL
	shlb	bl, $1
	mov	POS, cs:rowtab(bx)
	movb	bl, COL
	cmpb	MM_NCOL(bp), $40
	jne	0f
	shlb	bl, $1
0:	add	POS, bx
	mov	ax, MM_CURSE(bp)
	and	ax, MM_VIS(bp)
#ifdef TECMAR
	xor	es:[NHB*6](POS), ax	/ turn cursor on
	xor	es:[NHB*6]+BANKSZ(POS), ax
	xor	es:[NHB*7](POS), ax
	xor	es:[NHB*7]+BANKSZ(POS), ax
#else
	xor	es:[NHB*3](POS), ax	/ turn cursor on
	xor	es:[NHB*3]+BANKSZ(POS), ax
#endif
	pop	bx
	pop	es
	pop	ds
	mov	MM_FUNC(bp), bx
	movb	MM_ROW(bp), ROW		/ save row,column
	movb	MM_COL(bp), COL
	mov	MM_POS(bp), POS		/ save position

	mov	dx, $MSR		/ enable video display
	movb	al, $0x1A
	outb	dx, al
	mov	mmvcnt_, $480		/ 480 seconds before video disabled

	mov	bp, sp
	mov	bx, 8(bp)
	mov	ax, cx			/ Return the residual count.
	xchg	cx, IO_IOC(bx)
	sub	cx, IO_IOC(bx)
	add	IO_BASE(bx), cx
	pop	bp
	pop	di
	pop	si
	ret

////////
/
/ mminit - initialize screen
/
////////

mminit:	movb	ss:mmesc_, $CLOWER		/ schedule keyboard initialization
	mov	dx, $MSR		/ disable video display
	movb	al, $0x12
	outb	dx, al

	push	cx			/ program registers, last to first
	mov	bx, $15
	mov	dx, MM_PORT(bp)
0:	movb	al, bl
	outb	dx, al
	inc	dx
	movb	al, cs:crtdata(bx)
	outb	dx, al
	dec	dx
	dec	bx
	jge	0b
	pop	cx

	mov	dx, $CSR
	movb	al, $0x0F
	outb	dx, al

	mov	dx, $XMSR
#ifdef TECMAR
	movb	al, $31
#else
	movb	al, $0
#endif
	outb	dx, al

/	mov	dx, $XMSR
0:	inb	al, dx		/ wait for vertical retrace
	andb	al, $8
	je	0b

	mov	dx, $MSR
	movb	al, $0x1A	/ video display on
	outb	dx, al

	mov	MM_VIS(bp), $-1
	mov	MM_MASK(bp), $0xAAAA
	mov	MM_FLIP(bp), $0
	mov	MM_ULINE(bp), $0
	mov	MM_CURSE(bp), $0x00ff
	movb	MM_WRAP(bp), $1
	movb	MM_NCOL(bp), $80
	subb	COL, COL
	movb	ROW, MM_IBROW(bp)
	movb	MM_BROW(bp), ROW
	movb	bl, MM_IEROW(bp)
	movb	MM_EROW(bp), bl
	sub	bx, bx
	movb	MM_N1(bp), $2
	jmp	mm_ed

////////
/
/ mmbell - schedule beep
/
////////

mmbell:	movb	ss:mmbeeps_, $-1
	jmp	eval

////////
/
/ mmspec - pass special characters on to keyboard routine(s).
/
////////

mmspec:	movb	ss:mmesc_, al
	jmp	eval

////////
/
/ mm_cnl - cursor next line
/
/	Moves the active position to the first column of the next display line.
/	Scrolls the active display if necessary.
/	Returns to mmwrite() to allow flow control on each output line.
/
////////

mm_cnl:	subb	COL, COL
	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jle	repos
	movb	ROW, MM_EROW(bp)
/	jmp	scrollup

////////
/
/ scrollup - scroll display upwards
/
////////

scrollup:
	push	ds
	push	si
	push	cx
	mov	ds, MM_BASE(bp)
	movb	bl, MM_BROW(bp)
	shlb	bl, $1
	mov	di, cs:rowtab(bx)
	mov	si, cs:rowtab+2(bx)
	movb	bl, ROW
	shlb	bl, $1
	mov	cx, cs:rowtab(bx)
	push	cx
	sub	cx, di
	shr	cx, $1
	push	si
	push	di
	push	cx
	rep
	movsw
	pop	cx
	pop	di
	pop	si
	add	si, $BANKSZ
	add	di, $BANKSZ
	rep
	movsw
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	pop	di
	push	di
	mov	cx, $NRB4
	rep
	stosw
	pop	di
	add	di, $BANKSZ
	mov	cx, $NRB4
	rep
	stosw
	pop	cx
	pop	si
	pop	ds
	jmp	ewait

////////
/
/ repos - reposition cursor
/
////////

repos:	movb	bl, ROW			/ reposition to ROW and COL
	shlb	bl, $1
	mov	POS, cs:rowtab(bx)
	movb	bl, COL
	cmpb	MM_NCOL(bp), $40
	jne	0f
	shlb	bl, $1
0:	add	POS, bx
/	jmp	eval

////////
/
/ eval - evaluate input character
/
////////

eval:	jcxz	ewait0
	dec	cx				/ evaluate next char
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

ewait0:	call	exit
	jcxz	0b
	dec	cx
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

////////
/
/ mmputc - put character on screen
/
////////

mmputc:	cmpb	MM_NCOL(bp), $40
	jne	putc8


putc16:	push	ds
	push	si
	subb	ah, ah
	shlb	al, $1
	shl	ax, $1
	shl	ax, $1
	shl	ax, $1
	add	ax, $fontw_
	mov	si, ax
	mov	ax, cs
	mov	ds, ax
	mov	bx, $BANKSZ-2

	lodsw				/ row 0
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	stosw
	push	di			/ save position for next char
#ifdef TECMAR
	mov	es:(bx,di), ax
	add	di, $78
#endif
	lodsw				/ row 1
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
#ifdef TECMAR
	stosw
#endif
	mov	es:(bx,di), ax
	add	di, $78

	lodsw				/ row 2
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	stosw
#ifdef TECMAR
	mov	es:(bx,di), ax
	add	di, $78
#endif

	lodsw				/ row 3
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
#ifdef TECMAR
	stosw
#endif
	mov	es:(bx,di), ax
	add	di, $78

	lodsw				/ row 4
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	stosw
#ifdef TECMAR
	mov	es:(bx,di), ax
	add	di, $78
#endif

	lodsw				/ row 5
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
#ifdef TECMAR
	stosw
#endif
	mov	es:(bx,di), ax
	add	di, $78

	lodsw				/ row 6
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	stosw
#ifdef TECMAR
	mov	es:(bx,di), ax
	add	di, $78
#endif
	lodsw				/ row 7
	or	ax, MM_ULINE(bp)
	xor	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
#ifdef TECMAR
	stosw
#endif
	mov	es:(bx,di), ax


	pop	di			/ restore position for next char
	pop	si
	pop	ds

	sub	bx, bx
	incb	COL
	cmpb	COL, MM_NCOL(bp)
	jge	0f
	jcxz	ewait1
	dec	cx
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

0:	subb	COL, COL
	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jg	0f
	jmp	repos

0:	movb	ROW, MM_EROW(bp)
	jmp	scrollup

ewait1:	jmp	ewait

putc8:	push	ds
	push	si
	subb	ah, ah
	shlb	al, $1
	shl	ax, $1
	shl	ax, $1
	add	ax, $0xFA6E
	mov	si, ax
	mov	ax, $0xF000
	mov	ds, ax
	mov	bx, $BANKSZ-1

	lodsw
	xor	ax, MM_FLIP(bp)
	stosb				/ row 0
	push	di			/ save position for next char
#ifdef TECMAR
	movb	es:(bx,di), al
	movb	es:79(di), ah		/ row 1
	movb	es:80(bx,di), ah
	add	di, $79+80
#else
	movb	es:(bx,di), ah		/ row 1
	add	di, $79
#endif

	lodsw
	xor	ax, MM_FLIP(bp)
	stosb				/ row 2
#ifdef TECMAR
	movb	es:(bx,di), al
	movb	es:79(di), ah
	movb	es:80(bx,di), ah	/ row 3
	add	di, $79+80
#else
	movb	es:(bx,di), ah		/ row 3
	add	di, $79
#endif

	lodsw
	xor	ax, MM_FLIP(bp)
	stosb				/ row 4
#ifdef TECMAR
	movb	es:(bx,di), al
	movb	es:79(di), ah		/ row 5
	movb	es:80(bx,di), ah
	add	di, $79+80
#else
	movb	es:(bx,di), ah		/ row 5
	add	di, $79
#endif

	lodsw
	orb	ah, MM_ULINE(bp)
	xor	ax, MM_FLIP(bp)
	stosb				/ row 6
#ifdef TECMAR
	movb	es:(bx,di), al
	movb	es:79(di), ah
	movb	es:80(bx,di), ah
#else
	movb	es:(bx,di), ah		/ row 7
#endif

	pop	di			/ restore position for next char
	pop	si
	pop	ds

	sub	bx, bx
	incb	COL
	cmpb	COL, MM_NCOL(bp)
	jge	0f
	jcxz	ewait
	dec	cx
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

0:	subb	COL, COL
	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jg	0f
	jmp	repos

0:	movb	ROW, MM_EROW(bp)
	jmp	scrollup

////////
/
/ Ewait - wait for next input char to evaluate
/
////////

ewait:	call	exit
	jcxz	ewait
	dec	cx
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

////////
/
/ mm_cr - carriage return
/
/	Moves the active position to first position of current display line.
/
////////

mm_cr:	subb	COL, COL
	movb	bl, ROW
	shlb	bl, $1
	mov	POS, cs:rowtab(bx)
	jcxz	ewait
	dec	cx
	lodsb
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:asctab(bx)

////////
/
/ mm_cub - cursor backwards
/
////////

mm_cub:	decb	COL
	jge	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
	decb	ROW
	cmpb	ROW, MM_BROW(bp)
	jge	0f
	subb	COL, COL
	movb	ROW, MM_BROW(bp)
0:	jmp	repos

////////
/
/ Esc state - entered when last char was ESC - transient state.
/
////////

0:	call	exit
mm_esc:	jcxz	0b
	dec	cx
	lodsb
	movb	MM_N1(bp), ZERO
	movb	MM_N2(bp), ZERO
	movb	bl, al
	shlb	bl, $1
	ijmp	cs:esctab(bx)

////////
/
/ Csi_n1 state - entered when last two chars were ESC [
/
/	Action:	Evaluates numeric chars as numeric parameter 1.
/
////////

0:	call	exit
csi_n1:	jcxz	0b
	dec	cx
	lodsb
	cmpb	al, $SEMIC
	je	csi_n2
	movb	bl, al
	subb	bl, $AZERO
	cmpb	bl, $9
	ja	csival
	shlb	MM_N1(bp), $1	/ n1 * 2
	movb	al, MM_N1(bp)	/ n1 * 2
	shlb	al, $1		/ n1 * 4
	shlb	al, $1		/ n1 * 8
	addb	al, MM_N1(bp)	/ n1 * 10
	addb	al, bl		/ n1 * 10 + digit
	movb	MM_N1(bp), al	/ n1 = (n1 * 10) + digit
	jmp	csi_n1

////////
/
/ Csi_n2 state - entered after input sequence ESC [ n ;
/
////////

0:	call	exit
csi_n2:	jcxz	0b
	dec	cx
	lodsb
	movb	bl, al
	subb	bl, $AZERO
	cmpb	bl, $9
	ja	csival
	shlb	MM_N2(bp), $1	/ n2 * 2
	movb	al, MM_N2(bp)	/ n2 * 2
	shlb	al, $1		/ n2 * 4
	shlb	al, $1		/ n2 * 8
	addb	al, MM_N2(bp)	/ n2 * 10
	addb	al, bl		/ n2 * 10 + digit
	movb	MM_N2(bp), al	/ n2 = (n2 * 10) + digit
	jmp	csi_n2

csival:	movb	bl, al
	shlb	bl, $1
	ijmp	cs:csitab(bx)

////////
/
/ Csi_gt state - entered after input sequence ESC [ >
/	
////////

0:	call	exit
csi_gt:	jcxz	0b
	dec	cx
	lodsb
	movb	bl, al
	subb	bl, $AZERO
	cmpb	bl, $9
	ja	0f
	shlb	MM_N1(bp), $1	/ n1 * 2
	movb	al, MM_N1(bp)	/ n1 * 2
	shlb	al, $1		/ n1 * 4
	shlb	al, $1		/ n1 * 8
	addb	al, MM_N1(bp)	/ n1 * 10
	addb	al, bl		/ n1 * 10 + digit
	movb	MM_N1(bp), al	/ n1 = (n1 * 10) + digit
	jmp	csi_gt

0:	cmpb	al, $HLOWER
	je	mm_cgh
	cmpb	al, $LLOWER
	je	mm_cgl
	jmp	eval

////////
/
/ Csi_q state - entered after input sequence ESC [ ?
/	
////////

0:	call	exit
csi_q:	jcxz	0b
	dec	cx
	lodsb
	movb	bl, al
	subb	bl, $AZERO
	cmpb	bl, $9
	ja	0f
	shlb	MM_N1(bp), $1	/ n1 * 2
	movb	al, MM_N1(bp)	/ n1 * 2
	shlb	al, $1		/ n1 * 4
	shlb	al, $1		/ n1 * 8
	addb	al, MM_N1(bp)	/ n1 * 10
	addb	al, bl		/ n1 * 10 + digit
	movb	MM_N1(bp), al	/ n1 = (n1 * 10) + digit
	jmp	csi_q

0:	cmpb	al, $HLOWER
	je	mm_cqh
	cmpb	al, $LLOWER
	je	mm_cql
	jmp	eval

////////
/
/ mm_cbt - cursor backward tabulation
/
/	Moves the active position horizontally in the backward direction
/	to the preceding in a series of predetermined positions.
/
////////

mm_cbt:	orb	COL, $7			/ calculate next tab stop
	incb	COL
	subb	COL, $16		/ step back two tab positions
	jg	0f
	subb	COL, COL		/ cannot step past column 0
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_cgh - process ESC [ > N1 h escape sequence
/
/	Recognized sequences:	ESC [ > 13 h	-- Set CRT saver enabled.
/
////////

mm_cgh:	cmpb	MM_N1(bp), $13
	jne	0f
	mov	ss:mmcrtsav_, $1
0:	jmp	eval

////////
/
/ mm_cgl - process ESC [ > N1 l escape sequence
/
/	Recognized sequences:	ESC [ > 13 l	-- Reset CRT saver.
/
////////

mm_cgl:	cmpb	MM_N1(bp), $13
	jne	0f
	mov	ss:mmcrtsav_, $0
0:	jmp	eval

////////
/
/ mm_cha - cursor horizontal absolute
/
/	Advances the active position forward or backward along the active line
/	to the character position specified by the parameter.
/	A parameter value of zero or one moves the active position to the
/	first character position of the active line.
/	A parameter value of N moves the active position to character position
/	N of the active line.
/
////////

mm_cha:	movb	COL, MM_N1(bp)
	decb	COL
	jge	0f
	subb	COL, COL
0:	cmpb	COL, MM_NCOL(bp)
	jb	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos			/ reposition cursor


////////
/
/ mm_cht - cursor horizontal tabulation
/
/	Advances the active position horizontally to the next or following
/	in a series of predetermined positions.
/
////////

mm_cht:	mov	bx, $BANKSZ
	push	cx
	push	si
	sub	cx, cx
	movb	cl, COL
	orb	cl, $7
	incb	cl
	subb	cl, COL
	addb	COL, cl

	mov	si, MM_MASK(bp)
	cmpb	MM_NCOL(bp), $80
	jne	0f
	mov	si, $-1
	inc	cx
	shr	cx, $1

0:	mov	ax, MM_FLIP(bp)
	and	ax, si
#ifdef TECMAR
	stosw				/ row 0
	mov	es:[NHB*0]-2(di,bx), ax
	mov	es:[NHB*1]-2(di), ax	/ row 1
	mov	es:[NHB*1]-2(di,bx), ax
	mov	es:[NHB*2]-2(di), ax	/ row 2
	mov	es:[NHB*2]-2(di,bx), ax
	mov	es:[NHB*3]-2(di), ax	/ row 3
	mov	es:[NHB*3]-2(di,bx), ax
	mov	es:[NHB*4]-2(di), ax	/ row 4
	mov	es:[NHB*4]-2(di,bx), ax
	mov	es:[NHB*5]-2(di), ax	/ row 5
	mov	es:[NHB*5]-2(di,bx), ax
	mov	es:[NHB*6]-2(di), ax	/ row 6
	mov	es:[NHB*6]-2(di,bx), ax
	or	ax, MM_ULINE(bp)
	and	ax, si
	mov	es:[NHB*7]-2(di), ax	/ row 7
	mov	es:[NHB*7]-2(di,bx), ax
#else
	stosw				/ row 0
	mov	es:[NHB*0]-2(di,bx), ax	/ row 1
	mov	es:[NHB*1]-2(di), ax	/ row 2
	mov	es:[NHB*1]-2(di,bx), ax	/ row 3
	mov	es:[NHB*2]-2(di), ax	/ row 4
	mov	es:[NHB*2]-2(di,bx), ax	/ row 5
	mov	es:[NHB*3]-2(di), ax	/ row 6
	or	ax, MM_ULINE(bp)
	and	ax, si
	mov	es:[NHB*3]-2(di,bx), ax	/ row 7
#endif
	loop	0b
	sub	bx, bx
	pop	si
	pop	cx
	cmpb	COL, MM_NCOL(bp)
	jl	0f
	subb	COL, MM_NCOL(bp)
	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jle	0f
	movb	ROW, MM_EROW(bp)
	jmp	scrollup
0:	jmp	repos

////////
/
/ mm_cpl - cursor preceding line
/
/	Moves the active position to the first position of the preceding
/	display line.
/
////////

mm_cpl:	subb	COL, COL
	decb	ROW
	cmpb	ROW, MM_BROW(bp)
	jnb	0f
	movb	ROW, MM_BROW(bp)
	jmp	scrolldown
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_cqh - process ESC [ ? N1 h escape sequence
/
/	Recognized sequences:	ESC [ ? 7 h	-- Set wraparound.
/
////////

mm_cqh:
	cmpb	MM_N1(bp), $7		/ Wraparound.
	jne	0f
	movb	MM_WRAP(bp), $1
0:	jmp	eval

////////
/
/ mm_cql - process ESC [ ? N1 l escape sequence
/
/	Recognized sequences:	ESC [ ? 7 l	-- Reset wraparound.
/
////////

mm_cql:
	cmpb	MM_N1(bp), $7		/ No wraparound.
	jne	0f
	movb	MM_WRAP(bp), $0
0:	jmp	eval

////////
/
/ mm_cud - cursor down
/
/	Moves the active position downward without altering the
/	horizontal position.
/
////////

mm_cud:	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jna	0f
	movb	ROW, MM_EROW(bp)
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_cuf - cursor forward
/
/	Moves the active position in the forward direction.
/
////////

mm_cuf:	incb	COL
	cmpb	COL, MM_NCOL(bp)
	jb	0f
	subb	COL, MM_NCOL(bp)
	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jna	0f
	movb	ROW, MM_EROW(bp)
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos

////////
/
/ mm_cup - cursor position
/
/	Moves the active position to the position specified by two parameters.
/	The first parameter (mm_n1) specifies the vertical position (MM_ROW(bp)).
/	The second parameter (mm_n2) specifies the horizontal position (MM_COL(bp)).
/	A parameter value of 0 or 1 for the first or second parameter
/	moves the active position to the first line or column in the
/	display respectively.
/
////////

mm_cup:	movb	ROW, MM_N1(bp)
	decb	ROW
	jg	0f
	subb	ROW, ROW
0:	addb	ROW, MM_BROW(bp)
	cmpb	ROW, MM_EROW(bp)
	jb	0f
	movb	ROW, MM_EROW(bp)
0:	movb	COL, MM_N2(bp)
	decb	COL
	jg	0f
	subb	COL, COL
0:	cmpb	COL, MM_NCOL(bp)
	jb	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_cuu - cursor up
/
/	Moves the active position upward without altering the horizontal
/	position.
/
////////

mm_cuu:	decb	ROW
	cmpb	ROW, MM_BROW(bp)
	jge	0f
	movb	ROW, MM_BROW(bp)
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_dl - delete line
/
/	Removes the contents of the active line.
/	The contents of all following lines are shifted in a block
/	toward the active line.
/
////////

mm_dl:	push	ds
	push	si
	push	cx
	mov	ds, MM_BASE(bp)
	movb	bl, ROW
	shlb	bl, $1
	mov	di, cs:rowtab(bx)
	mov	si, cs:rowtab+2(bx)
	movb	bl, MM_EROW(bp)
	shlb	bl, $1
	mov	cx, cs:rowtab(bx)
	sub	cx, di
	jle	0f
	shr	cx, $1
	push	si
	push	di
	push	cx
	rep
	movsw
	pop	cx
	pop	di
	pop	si
	add	si, $BANKSZ
	add	di, $BANKSZ
	rep
	movsw
	mov	di, cs:rowtab(bx)
	push	di
	mov	cx, $NRB4
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	rep
	stosw
	pop	di
	add	di, $BANKSZ
	mov	cx, $NRB4
	rep
	stosw
	subb	COL, COL
0:	pop	cx
	pop	si
	pop	ds
	jmp	repos

////////
/
/ mm_dmi - disable manual input
/
/	Set flag preventing keyboard input.
/
////////

mm_dmi:
	mov	ss:islock_, $1
	jmp	eval

////////
/
/ mm_ea - erase in area
/
/	Erase some or all of the characters in the currently active area
/	according to the parameter:
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase all of active area
/
////////

mm_ea:	movb	al, MM_N1(bp)
	cmpb	al, $0
	jne	0f
	movb	bl, MM_EROW(bp)
	jmp	mm_e0
0:	cmpb	al, $1
	jne	0f
	movb	bl, MM_BROW(bp)
	jmp	mm_e1
0:	subb	COL, COL
	movb	ROW, MM_BROW(bp)
	movb	bl, ROW
	shlb	bl, $1
	mov	POS, cs:rowtab(bx)
	movb	bl, MM_EROW(bp)
	subb	bl, ROW
	jmp	mm_e2

////////
/
/ mm_ed - erase in display
/
/	Erase some or all of the characters in the display according to the
/	parameter
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase all of display
/
////////

mm_ed:	movb	al, MM_N1(bp)
	cmpb	al, $0
	jne	0f
	movb	bl, MM_LROW(bp)
	jmp	mm_e0
0:	cmpb	al, $1
	jne	0f
	subb	bl, bl
	jmp	mm_e1
0:	subb	COL, COL
	movb	ROW, MM_BROW(bp)
	sub	POS, POS
	movb	bl, MM_LROW(bp)
	jmp	mm_e2

////////
/
/ mm_el - erase in line
/
/	Erase some or all of the characters in the line according to the
/	parameter:
/		0 - erase from active position to end inclusive (default)
/		1 - erase from start to active position inclusive
/		2 - erase entire line
/
////////

mm_el:	movb	al, MM_N1(bp)
	movb	bl, ROW
	cmpb	al, $0
	je	mm_e0
	cmpb	al, $1
	je	mm_e1
	shlb	bl, $1
	mov	POS, cs:rowtab(bx)
	subb	COL, COL
	subb	bl, bl
/	jmp	mm_e2

////////
/
/ mm_e2 - erase from POS for BL rows (minimum 1 row)
/
////////

mm_e2:	push	cx
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
0:	mov	cx, $NRB4
	rep
	stosw
	add	di, $BANKSZ-NRB2
	mov	cx, $NRB4
	rep
	stosw
	sub	di, $BANKSZ
	decb	bl
	jge	0b
	pop	cx
	jmp	repos

////////
/
/ mm_e1 - erase from row BL to current cursor position.
/
////////

mm_e1:	push	dx
	push	cx
	push	di
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	shlb	bl, $1
	mov	di, cs:rowtab(bx)
	movb	bl, ROW
	shlb	bl, $1
	mov	cx, cs:rowtab(bx)
	sub	cx, di
	jle	0f
	shr	cx, $1
	push	di
	push	cx
	rep
	stosw
	pop	cx
	pop	di
	add	di, $BANKSZ
	rep
	stosw
0:	pop	cx
	movb	bl, ROW
	shlb	bl, $1
	mov	di, cs:rowtab(bx)
	sub	cx, di
	jle	0f

	mov	dx, di
	mov	bx, cx
	rep			/ erase row 0
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 1
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 2
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 3
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 4
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 5
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 6
	stosb
#ifdef TECMAR
	add	dx, $80
	mov	di, dx
	mov	cx, bx
	rep			/ erase row 7
	stosb
	add	dx, $BANKSZ-560
#else
	add	dx, $BANKSZ-240
#endif
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 0
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 1
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 2
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 3
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 4
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 5
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 6
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 7
	stosb
1:	sub	bx, bx
	pop	cx
	pop	dx
	jmp	repos

////////
/
/ mm_e0 - erase from current cursor position to row BL inclusive.
/
////////

mm_e0:	push	dx
	push	cx
	push	di
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	shlb	bl, $1
	mov	cx, cs:rowtab+2(bx)
	movb	bl, ROW
	shlb	bl, $1
	mov	di, cs:rowtab+2(bx)
	sub	cx, di
	jle	0f
	shr	cx, $1
	push	di
	push	cx
	rep
	stosw
	pop	cx
	pop	di
	add	di, $BANKSZ
	rep
	stosw
0:	pop	di
	sub	cx, cx
	movb	cl, MM_NCOL(bp)
	subb	cl, COL
	cmpb	MM_NCOL(bp), $40
	jne	0f
	shlb	cl, $1
0:	mov	dx, di
	mov	bx, cx
	rep			/ erase row 0
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 1
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 2
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 3
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 4
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 5
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 6
	stosb
#ifdef TECMAR
	add	dx, $80
	mov	di, dx
	mov	cx, bx
	rep			/ erase row 7
	stosb
	add	dx, $BANKSZ-560
#else
	add	dx, $BANKSZ-240
#endif
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 0
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 1
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 2
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 3
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 4
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 5
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#ifdef TECMAR
	rep			/ erase row 6
	stosb
	add	dx, $80
	mov	di, dx
	mov	cx, bx
#endif
	rep			/ erase row 7
	stosb
1:	sub	bx, bx
	pop	cx
	pop	dx
	jmp	repos

////////
/
/ mm_emi - enable manual input
/
/	Clear flag preventing keyboard input.
/
////////

mm_emi:
	mov	ss:islock_, $0
	jmp	eval

////////
/
/ mm_il - insert line
/
/	Insert a erased line at the active line by shifting the contents
/	of the active line and all following lines away from the active line.
/	The contents of the last line in the scrolling region are removed.
/
////////

scrolldown:
mm_il:	push	ds
	push	si
	push	cx
	mov	ds, MM_BASE(bp)
	movb	bl, MM_EROW(bp)
	shlb	bl, $1
	mov	si, cs:rowtab(bx)
	mov	cx, si
	sub	si, $2
	mov	di, cs:rowtab+2(bx)
	sub	di, $2
	movb	bl, ROW
	shlb	bl, $1
	sub	cx, cs:rowtab(bx)
	jle	0f
	shr	cx, $1
	push	si
	push	di
	push	cx
	std
	rep
	movsw
	pop	cx
	pop	di
	pop	si
	add	si, $BANKSZ
	add	di, $BANKSZ
	rep
	movsw
	mov	di, cs:rowtab(bx)
	push	di
	mov	cx, $NRB4
	mov	ax, MM_FLIP(bp)
	and	ax, MM_MASK(bp)
	cld
	rep
	stosw
	pop	di
	add	di, $BANKSZ
	mov	cx, $NRB4
	rep
	stosw
	subb	COL, COL
0:	pop	cx
	pop	si
	pop	ds
	jmp	repos

////////
/
/ mm_hpa - horizontal position absolute
/
/	Moves the active position within the active line to the position
/	specified by the parameter.  A parameter value of zero or one
/	moves the active position to the first position of the active line.
/
////////

mm_hpa:	movb	COL, MM_N1(bp)
	decb	COL
	jg	0f
	subb	COL, COL
0:	cmpb	COL, MM_NCOL(bp)
	jb	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_hpr - horizontal position relative
/
/	Moves the active position forward the number of positions specified
/	by the parameter.  A parameter value of zero or one indicates a
/	single-position move.
/
////////

mm_hpr:	movb	al, MM_N1(bp)
	orb	al, al
	jne	0f
	incb	al
0:	addb	COL, al
	cmpb	COL, MM_NCOL(bp)
	jb	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_hvp - horizontal and vertical position
/
/	Moves the active position to the position specified by two parameters.
/	The first parameter specifies the vertical position (MM_ROW(bp)).
/	The second parameter specifies the horizontal position (MM_COL(bp)).
/	A parameter value of zero or one moves the active position to the
/	first line or column in the display.
/
////////

mm_hvp:	movb	ROW, MM_N1(bp)
	decb	ROW
	jg	0f
	subb	ROW, ROW
0:	cmpb	ROW, MM_LROW(bp)
	jna	0f
	movb	ROW, MM_LROW(bp)
0:	movb	COL, MM_N2(bp)
	decb	COL
	jg	0f
	subb	COL, COL
0:	cmpb	COL, MM_NCOL(bp)
	jb	0f
	movb	COL, MM_NCOL(bp)
	decb	COL
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_ind - index
/
/	Causes the active position to move downward one line without changing
/	the horizontal position.  Scrolling occurs if below scrolling region.
/
////////

mm_ind:	incb	ROW
	cmpb	ROW, MM_EROW(bp)
	jg	0f
	jmp	repos
0:	movb	ROW, MM_EROW(bp)
	jmp	scrollup

////////
/
/ mm_new - save cursor position
/
////////

mm_new:	movb	MM_SCOL(bp), COL
	movb	MM_SROW(bp), ROW
	jmp	eval

////////
/
/ mm_old - restore old cursor position
/
////////

mm_old:	movb	COL, MM_SCOL(bp)
	movb	ROW, MM_SROW(bp)
	jmp	repos

////////
/
/ mm_ri - reverse index
/
/	Moves the active position to the same horizontal position on the
/	preceding line.  Scrolling occurs if above scrolling region.
/
////////

mm_ri:	decb	ROW
	cmpb	ROW, MM_BROW(bp)
	jge	0f
	movb	ROW, MM_BROW(bp)
	jmp	scrolldown
0:	jmp	repos

////////
/
/ mm_scr - select cursor rendition
/
/	Invokes the cursor rendition specified by the parameter.
/
/	Recognized renditions are:	0 - cursor visible
/					1 - cursor invisible
////////

mm_scr:	decb	MM_N1(bp)
	je	0f
	jg	1f
	mov	MM_VIS(bp), $-1
	jmp	eval

0:	mov	MM_VIS(bp), $0
1:	jmp	eval

////////
/
/ mm_sgr - select graphic rendition
/
/	Invokes the graphic rendition specified by the parameter.
/	All following characters in the data stream are rendered
/	according to the parameters until the next occurrence of
/	SGR in the data stream.
/
/	Recognized renditions are:	1 - high intensity
/					4 - underline
/					7 - reverse video
/
////////

mm_sgr:	movb	al, MM_N1(bp)
	cmpb	al, $0
	jne	0f
	mov	MM_MASK(bp), $0xAAAA
	mov	MM_FLIP(bp), $0
	mov	MM_ULINE(bp), $0
	jmp	1f
0:	cmpb	al, $1		/ bold
	jne	0f
	mov	MM_MASK(bp), $-1
	jmp	1f
0:	cmpb	al, $4		/ underline
	jne	0f
	mov	MM_ULINE(bp), $0xFFFF
	jmp	1f
0:	cmpb	al, $7		/ reverse video
	jne	0f
	mov	MM_FLIP(bp), $-1
	jmp	1f
0:
1:	jmp	eval

////////
/
/ mm_so - stand out - enter 40 column mode
/
////////

mm_so:	cmpb	MM_NCOL(bp), $80
	jne	0f
	movb	MM_NCOL(bp), $40
	incb	COL
	shrb	COL, $1
0:	mov	MM_CURSE(bp), $0xffff
	jmp	repos

////////
/
/ mm_si - stand in - enter 80 column mode
/
////////

mm_si:	cmpb	MM_NCOL(bp), $40
	jne	0f
	movb	MM_NCOL(bp), $80
	shlb	COL, $1
0:	mov	MM_CURSE(bp), $0x00ff
	jmp	repos

////////
/
/ mm_ssr - set scrolling region
/
////////

mm_ssr:	movb	al, MM_N1(bp)
	decb	al
	jge	0f
	subb	al, al
0:	cmpb	al, MM_LROW(bp)
	ja	1f
	movb	bl, MM_N2(bp)
	decb	bl
	jge	0f
	subb	bl, bl
0:	cmpb	bl, MM_LROW(bp)
	ja	1f
	cmpb	al, bl
	ja	1f
	movb	MM_BROW(bp), al
	movb	MM_EROW(bp), bl
	movb	ROW, al
	subb	COL, COL
1:	jmp	repos

////////
/
/ mm_vpa - vertical position absolute
/
/	Moves the active position to the line specified by the parameter
/	without changing the horizontal position.
/	A parameter value of 0 or 1 moves the active position vertically
/	to the first line.
/
////////

mm_vpa:	movb	ROW, MM_N1(bp)
	decb	ROW
	jg	0f
	subb	ROW, ROW
0:	cmpb	ROW, MM_LROW(bp)
	jna	0f
	movb	ROW, MM_LROW(bp)
0:	jmp	repos			/ reposition cursor

////////
/
/ mm_vpr - vertical position relative
/
/	Moves the active position downward the number of lines specified
/	by the parameter without changing the horizontal position.
/	A parameter value of zero or one moves the active position
/	one line downward.
/
////////

mm_vpr:	movb	al, MM_N1(bp)
	orb	al, al
	jne	0f
	incb	al
0:	addb	ROW, al
	cmpb	ROW, MM_LROW(bp)
	jb	0f
	movb	ROW, MM_LROW(bp)
0:	jmp	repos			/ reposition cursor

////////
/
/ asctab - table of functions indexed by ascii characters
/
////////

asctab:	.word	eval,	eval,	eval,	eval	/* NUL  SOH  STX  ETX  */
	.word	eval,	eval,	eval,	mmbell	/* EOT  ENQ  ACK  BEL  */
	.word	mm_cub,	mm_cht,	mm_cnl,	mm_ind	/* BS   HT   LF   VT  */
	.word	eval,	mm_cr,	mm_so,	mm_si	/* FF   CR   SO   SI  */
	.word	eval,	eval,	eval,	eval	/* DLE  DC1  DC2  DC3 */
	.word	eval,	eval,	eval,	eval	/* DC4  NAK  SYN  ETB  */
	.word	eval,	eval,	eval,	mm_esc	/* CAN  EM   SUB  ESC  */
	.word	eval,	eval,	eval,	eval	/* FS   GS   RS   US   */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/*   ! " # \040 - \043 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* $ % & quote \044 - \047 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* ( ) * + \050 - \053 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* , - . / \054 - \057 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* 0 1 2 3 \060 - \063 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* 4 5 6 7 \064 - \067 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* 8 9 : ; \070 - \073 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* < = > ? \074 - \077 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* @ A B C \100 - \103 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* D E F G \104 - \107 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* H I J K \110 - \113 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* L M N O \114 - \117 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* P Q R S \120 - \123 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* T U V W \124 - \127 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* X Y Z [ \130 - \133 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* \ ] ^ _ \134 - \137 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* ` a b c \140 - \143 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* d e f g \144 - \147 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* h i j k \150 - \153 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* l m n o \154 - \157 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* p q r s \160 - \163 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* t u v w \164 - \167 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* x y z { \170 - \173 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* | } ~ ? \174 - \177 */

////////
/
/ esctab - table of functions indexed by escape characters.
/
////////

esctab:	.word	mmputc,	mmputc,	mmputc,	mmputc	/* NUL  SOH  STX  ETX  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* EOT  ENQ  ACK  BEL  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* BS   HT   LF   VT  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* FF   CR   SO   SI  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* DLE  DC1  DC2  DC3 */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* DC4  NAK  SYN  ETB  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* CAN  EM   SUB  ESC  */
	.word	mmputc,	mmputc,	mmputc,	mmputc	/* FS   GS   RS   US   */
	.word	eval,	eval,	eval,	eval	/*   ! " # \040 - \043 */
	.word	eval,	eval,	eval,	eval	/* $ % & quote \044 - \047 */
	.word	eval,	eval,	eval,	eval	/* ( ) * + \050 - \053 */
	.word	eval,	eval,	eval,	eval	/* , - . / \054 - \057 */
	.word	eval,	eval,	eval,	eval	/* 0 1 2 3 \060 - \063 */
	.word	eval,	eval,	eval,	mm_new	/* 4 5 6 7 \064 - \067 */
	.word	mm_old,	eval,	eval,	eval	/* 8 9 : ; \070 - \073 */
	.word	eval,	mmspec,	mmspec,	eval	/* < = > ? \074 - \077 */
	.word	eval,	eval,	eval,	eval	/* @ A B C \100 - \103 */
	.word	mm_ind,	mm_cnl,	eval,	eval	/* D E F G \104 - \107 */
	.word	eval,	eval,	eval,	eval	/* H I J K \110 - \113 */
	.word	eval,	mm_ri,	eval,	eval	/* L M N O \114 - \117 */
	.word	eval,	eval,	eval,	eval	/* P Q R S \120 - \123 */
	.word	eval,	eval,	eval,	eval	/* T U V W \124 - \127 */
	.word	eval,	eval,	eval,	csi_n1	/* X Y Z [ \130 - \133 */
	.word	eval,	eval,	eval,	eval	/* \ ] ^ _ \134 - \137 */
	.word	mm_dmi,	eval,	mm_emi,	mminit	/* ` a b c \140 - \143 */
	.word	eval,	eval,	eval,	eval	/* d e f g \144 - \147 */
	.word	eval,	eval,	eval,	eval	/* h i j k \150 - \153 */
	.word	eval,	eval,	eval,	eval	/* l m n o \154 - \157 */
	.word	eval,	eval,	eval,	eval	/* p q r s \160 - \163 */
	.word	mmspec,	mmspec,	eval,	eval	/* t u v w \164 - \167 */
	.word	eval,	eval,	eval,	eval	/* x y z { \170 - \173 */
	.word	eval,	eval,	eval,	eval	/* | } ~ ? \174 - \177 */

////////
/
/ csitab - table of functions indexed by ESC [ characters.
/
////////

csitab:	.word	eval,	eval,	eval,	eval	/* NUL  SOH  STX  ETX  */
	.word	eval,	eval,	eval,	eval	/* EOT  ENQ  ACK  BEL  */
	.word	eval,	eval,	eval,	eval	/* BS   HT   LF   VT  */
	.word	eval,	eval,	eval,	eval	/* FF   CR   SO   SI  */
	.word	eval,	eval,	eval,	eval	/* DLE  DC1  DC2  DC3 */
	.word	eval,	eval,	eval,	eval	/* DC4  NAK  SYN  ETB  */
	.word	eval,	eval,	eval,	eval	/* CAN  EM   SUB  ESC  */
	.word	eval,	eval,	eval,	eval	/* FS   GS   RS   US   */
	.word	eval,	eval,	eval,	eval	/*   ! " # \040 - \043 */
	.word	eval,	eval,	eval,	eval	/* $ % & quote \044 - \047 */
	.word	eval,	eval,	eval,	eval	/* ( ) * + \050 - \053 */
	.word	eval,	eval,	eval,	eval	/* , - . / \054 - \057 */
	.word	eval,	eval,	eval,	eval	/* 0 1 2 3 \060 - \063 */
	.word	eval,	eval,	eval,	eval	/* 4 5 6 7 \064 - \067 */
	.word	eval,	eval,	eval,	eval	/* 8 9 : ; \070 - \073 */
	.word	eval,	eval,	csi_gt,	eval	/* < = > ? \074 - \077 */
	.word	eval,	mm_cuu,	mm_cud,	mm_cuf	/* @ A B C \100 - \103 */
	.word	mm_cub,	mm_cnl,	mm_cpl,	mm_cha	/* D E F G \104 - \107 */
	.word	mm_cup,	mm_cht,	mm_ed,	mm_el	/* H I J K \110 - \113 */
	.word	mm_il,	mm_dl,	eval,	mm_ea	/* L M N O \114 - \117 */
	.word	eval,	eval,	eval,	mm_ind	/* P Q R S \120 - \123 */
	.word	mm_ri,	eval,	eval,	eval	/* T U V W \124 - \127 */
	.word	eval,	eval,	mm_cbt,	eval	/* X Y Z [ \130 - \133 */
	.word	eval,	eval,	eval,	eval	/* \ ] ^ _ \134 - \137 */
	.word	mm_hpa,	mm_hpr,	eval,	eval	/* ` a b c \140 - \143 */
	.word	mm_vpa,	mm_vpr,	mm_hvp,	mm_cup	/* d e f g \144 - \147 */
	.word	eval,	eval,	eval,	eval	/* h i j k \150 - \153 */
	.word	eval,	mm_sgr,	eval,	eval	/* l m n o \154 - \157 */
	.word	eval,	eval,	mm_ssr,	eval	/* p q r s \160 - \163 */
	.word	eval,	eval,	mm_scr,	eval	/* t u v w \164 - \167 */
	.word	eval,	eval,	eval,	eval	/* x y z { \170 - \173 */
	.word	eval,	eval,	eval,	eval	/* | } ~ ? \174 - \177 */

////////
/
/ rowtab - array of offsets to each row
/
////////

rowtab:	.word	 0*NRB2,	 1*NRB2,	 2*NRB2,	 3*NRB2
	.word	 4*NRB2,	 5*NRB2,	 6*NRB2,	 7*NRB2
	.word	 8*NRB2,	 9*NRB2,	10*NRB2,	11*NRB2
	.word	12*NRB2,	13*NRB2,	14*NRB2,	15*NRB2
	.word	16*NRB2,	17*NRB2,	18*NRB2,	19*NRB2
	.word	20*NRB2,	21*NRB2,	22*NRB2,	23*NRB2
	.word	24*NRB2,	25*NRB2,	26*NRB2,	27*NRB2
	.word	28*NRB2,	29*NRB2,	30*NRB2,	31*NRB2
	.word	32*NRB2,	33*NRB2,	34*NRB2,	35*NRB2
	.word	36*NRB2,	37*NRB2,	38*NRB2,	39*NRB2
	.word	40*NRB2,	41*NRB2,	42*NRB2,	43*NRB2
	.word	44*NRB2,	45*NRB2,	46*NRB2,	47*NRB2
	.word	48*NRB2,	49*NRB2,	50*NRB2,	51*NRB2

////////
/
/ grread( dev, iop ) - read graphics display memory
/
////////

grread_:
	push	si
	push	di
	push	bp
	mov	bp, sp
	mov	bp, 10(bp)
	push	ds
	push	es
	cmp	IO_SEG(bp), $IOSYS
	je	0f
	mov	ax, uds_
	mov	es, ax
0:	mov	di, IO_BASE(bp)
	mov	ax, $VSEG
	mov	ds, ax

#ifndef TECMAR
	mov	ax, IO_SEEK(bp)
	add	ax, IO_IOC(bp)
	cmp	ax, $BANKSZ*2
	ja	done
#endif

	mov	ax, IO_SEEK(bp)
	sub	dx, dx
	mov	cx, $NHB
	div	cx
	mov	si, dx
	mov	bx, ax
	shr	ax, $1
	mul	cx
	mov	dx, si
	add	si, ax
	testb	bl, $1
	jne	read2

read1:	mov	cx, $NHB
	sub	cx, dx
	cmp	cx, IO_IOC(bp)
	jle	0f
	mov	cx, IO_IOC(bp)
	jcxz	done
0:	sub	IO_IOC(bp), cx
	add	IO_BASE(bp), cx
	push	si
	rep
	movsb
	pop	si
read2:	add	si, $BANKSZ
	mov	cx, $NHB
	sub	cx, dx
	cmp	cx, IO_IOC(bp)
	jle	0f
	mov	cx, IO_IOC(bp)
	jcxz	done
0:	sub	IO_IOC(bp), cx
	add	IO_BASE(bp), cx
	rep
	movsb
	sub	si, $BANKSZ
	sub	dx, dx
	jmp	read1

done:	pop	es
	pop	ds
	pop	bp
	pop	di
	pop	si
	ret

////////
/
/ grwrite( dev, iop ) - write graphics display memory
/
////////

grwrite_:
	push	si
	push	di
	push	bp
	mov	bp, sp
	mov	bp, 10(bp)
	push	ds
	push	es
	cmp	IO_SEG(bp), $IOSYS
	je	0f
	mov	ax, uds_
	mov	ds, ax
0:	mov	si, IO_BASE(bp)
	mov	ax, $VSEG
	mov	es, ax

#ifndef TECMAR
	mov	ax, IO_SEEK(bp)
	add	ax, IO_IOC(bp)
	cmp	ax, $BANKSZ*2
	ja	done
#endif

	mov	ax, IO_SEEK(bp)
	sub	dx, dx
	mov	cx, $NHB
	div	cx
	mov	di, dx
	mov	bx, ax
	shr	ax, $1
	mul	cx
	mov	dx, di
	add	di, ax
	testb	bl, $1
	jne	page2

page1:	mov	cx, $NHB
	sub	cx, dx
	cmp	cx, IO_IOC(bp)
	jle	0f
	mov	cx, IO_IOC(bp)
	jcxz	done
0:	sub	IO_IOC(bp), cx
	add	IO_BASE(bp), cx
	push	di
	rep
	movsb
	pop	di
page2:	add	di, $BANKSZ
	mov	cx, $NHB
	sub	cx, dx
	cmp	cx, IO_IOC(bp)
	jle	0f
	mov	cx, IO_IOC(bp)
	jcxz	done
0:	sub	IO_IOC(bp), cx
	add	IO_BASE(bp), cx
	rep
	movsb
	sub	di, $BANKSZ
	sub	dx, dx
	jmp	page1

////////
/
/ mm_voff()	-- Disable video display
/
////////
	.globl	mm_voff_
mm_voff_:
	mov	dx, $MSR
	movb	al, $0x12
	outb	dx, al
	ret

////////
/
/ mm_von()	-- Enable video display
/
////////
	.globl	mm_von_
mm_von_:
	mov	dx, $MSR		/ enable video display
	movb	al, $0x1A
	outb	dx, al
	mov	ss:mmvcnt_, $480	/ 480 seconds before video disabled
	ret
