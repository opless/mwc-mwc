/************************************************************************/
/*	Copyright (c) 1988, 1989 ARCHIVE Corporation			*/
/*	This program is an unpublished work fully protected by the	*/
/*	United States Copyright laws and is considered a trade secret	*/
/*	belonging to Archive Corporation.				*/
/************************************************************************/
/*	file: xla.orig.s		asm code for xl.c		*/
/*									*/
/*			13JUL88  14:00					*/
/*	Unix port	05/14/1989					*/
/************************************************************************/

/************************************************************************/
/*	386 version							*/
/************************************************************************/
	
/************************************************************************/
/*	request buffer structure					*/
/*	must match struc rb{} in xl.c					*/
/************************************************************************/
		.unixorder
rb:					/* request buffer structure	*/
rbnxt:		.long	0			/* ptr to next packet	*/
rbfun:		.byte	0			/* function		*/
rbsts:		.byte	0			/* status		*/
rbsgn:		.value	0			/* segment #		*/
rbadr:		.long	0			/* addr of buffer	*/
rbmap:		.long	0			/* bad sector map	*/
rbhed:		.byte	0			/* fdc head		*/
rbcyl:		.byte	0			/* fdc cylinder		*/
rbsct:		.byte	0			/* base fdc sector	*/
rbtrk:		.byte	0			/* tape track		*/
rbtps:		.byte	0			/*	segment		*/
rbidc:		.byte	0			/* read id cylinder	*/
rbids:		.byte	0			/*	   sct		*/
rbnbk:		.byte	0			/* # blocks		*/
rberc:		.byte	0			/* error count		*/
rbers:		.byte	0,0,0			/* error sectors	*/
rbtbl:		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
						/* parsed segment params*/
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
/rb	ends
/********************************************************/
/	data section					*/
/********************************************************/
		.data
/	tables must be on page boundary
/	which is why they are in _data
		.globl	xl_log2
		.globl	xl_alg2
		.globl	xl_mx02
		.globl	xl_mxc0
		.globl	xl_mxc3
xl_log2:	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/ecc tables
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/2
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/3
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/4
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/5
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/6
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/7
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/8
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/9
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/10
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/11
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/12
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/13
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/14
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/15
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/16
xl_alg2:	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/ecc tables
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/2
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/3
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/4
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/5
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/6
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/7
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/8
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/9
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/10
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/11
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/12
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/13
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/14
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/15
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/16
xl_mx02:	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/ecc tables
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/2
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/3
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/4
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/5
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/6
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/7
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/8
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/9
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/10
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/11
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/12
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/13
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/14
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/15
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/16
xl_mxc0:	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/ecc tables
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/2
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/3
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/4
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/5
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/6
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/7
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/8
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/9
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/10
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/11
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/12
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/13
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/14
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/15
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/16
xl_mxc3:	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/ecc tables
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/2
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/3
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/4
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/5
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/6
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/7
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/8
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/9
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/10
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/11
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/12
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/13
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/14
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/15
		.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0	/16
encj:		.long	0
chkj:		.long	0

		.globl	xl_encs
		.globl	xl_chks
xl_encs:	.byte	0
xl_chks:	.byte	0

		.globl	xl_tbl
		.globl	xl_bst
		.globl	xl_sct
		.globl	xl_cnt
		.globl	xl_ofs
xl_tbl:		.long	0		/* ptr to rb.tbl		*/
xl_bst:		.byte	0		/* base sector for segment	*/
xl_sct:		.byte	0		/* sector # for next chunk	*/
xl_cnt:		.long	0		/* # bytes to xfer next chunk	*/
xl_ofs:		.long	0		/* offset for next chunk	*/
prb:		.long	0		/* pointer to rb		*/
xl_nbk:		.byte	0		/* # blocks / segment		*/
					/* rb.tbl format:		*/
					/* bits 15-10 = dma count	*/
					/* bits  9-5  = dma offset >> 5	*/
					/* bits  4-0  = sector		*/

		.bss
/********************************************************/
/	code section					*/
/********************************************************/
		.text
		.align 4
/********************************************************/
/	xl_move	(src, dest, cnt)			*/
/********************************************************/
		.globl	xl_move
xl_move:
		push	%ebp		/* set up
		mov	%esp,%ebp
		push	%es
		push	%edi
		push	%esi
		push	%ds
		pop	%es
		mov	8(%ebp),%esi
		mov	12(%ebp),%edi
		mov	16(%ebp),%ecx
		cld
		jcxz	xl_mov0		/* br if no data
		shr	$2,%ecx		/* move longs
		repz
		movsl
		mov	16(%ebp),%ecx	/* move bytes (if any)
		and	$3,%ecx
		rep
		movsb
xl_mov0:
		pop	%esi		/* exit
		pop	%edi
		pop	%es
		pop	%ebp
		ret

/********************************************************/
/	xl_tbi	initialize table			*/
/********************************************************/
		.globl	xl_tbi
xl_tbi:
		push	%ebx
		push	%esi
/
		movl	$enc2,%eax		/initialize xl_encs
		subl	$enc1,%eax
		movb	%al,xl_encs
		movl	$chk2,%eax		/initialize xl_chks
		subl	$chk1,%eax
		movb	%al,xl_chks
/
		mov	$xl_alg2,%ebx		/init alog2 table
		movb	$01,%al
tbli0:
		movb	%al,(%ebx)
		inc	%ebx
		addb	%al,%al
		jnc	tbli1
		xorb	$0x87,%al
tbli1:
		cmpb	$01,%al
		jne	tbli0
		movb	%al,(%ebx)
/
		mov	$xl_alg2,%esi
		movb	$0x0ff,xl_log2
		xor	%eax,%eax
		xor	%ebx,%ebx
tbli2:
		movb	(%esi),%bl
		movb	%al,xl_log2(%ebx)
		inc	%esi
		inc	%eax
		cmpb	$255,%al
		jne	tbli2
/
		movl	$xl_mx02,%esi		/init mpy by 02 table
		mov	$0x00200,%ecx
tbli3:
		call	_mpy
		movb	%al,(%esi)
		inc	%esi
		incb	%cl
		jne	tbli3
/
		mov	$xl_mxc0,%esi		/init mpy by c0 table
		mov	$0x0c000,%ecx
tbli4:		call	_mpy
		movb	%al,(%esi)
		inc	%esi
		incb	%cl
		jne	tbli4
/
		movl	$xl_mxc3,%esi		/init mpy by c3 table
		mov	$0x0c300,%ecx
tbli5:
		call	_mpy
		movb	%al,(%esi)
		inc	%esi
		incb	%cl
		jne	tbli5
		pop	%esi
		pop	%ebx
		ret

/encm		macro	a,c,d,o
/		xor	o*1024(%edi),%e&a&x	;eax = new r0 bytes
/		xor	%e&c&x,%e&d&x		;set up edx for later
/		movb	%&a&l,%bl 		;multiply 4 bytes by c0
/		xorb	(%ebp,%ebx),%&c&l		; and xor to ecx
/		movb	%&a&h,%bl 		; ecx will be new r2
/		xorb	(%ebp,%ebx),%&c&h
/		rol	$16,%e&a&x
/		rol	$16,%e&c&x
/		movb	%&a&l,%bl
/		xorb	(%ebp,%ebx),%&c&l
/		movb	%&a&h,%bl
/		xorb	(%ebp,%ebx),%&c&h
/		rol	$16,%e&a&x
/		rol	$16,%e&c&x
/		xor	%e&c&x,%e&d&x		;edx = new r1
/		endm

/********************************************************/
/	xl_enc(bfr, nblks)   3/32  1024 byte blocks	*/
/********************************************************/
/
		.globl	xl_enc
xl_enc:
		enter	$0, $4		/ loop count is a local variable
		push	%ebx
		push	%edi
		push	%esi
		mov	8(%ebp),%edi	/edi = bfr
		mov	12(%ebp),%ebx	/ebx = nblks
		mov	%ebx,%eax 	/adjust edi
		dec	%eax
		shl	$10,%eax
		add	%eax,%edi
		mov	$32,%eax		/set encj according to # blocks
		subb	%bl,%al
		mulb	xl_encs
		add	$enc1,%eax
		mov	%eax,encj
		mov	$256,-4(%ebp) 	/set up for encode loop
enc0:
		mov	$xl_mxc0,%esi
		xor	%ebx,%ebx 	/edx ends up = r2
		xor	%edx,%edx 	/edx ends up = r2
		xor	%ecx,%ecx 	/ecx ends up = r1
		xor	%eax,%eax 	/eax ends up = r0
		jmp	*encj
/		.even
		.align	2
/
/		encode data
/
enc1:
		/encm	c,a,d,-31
		xor	-31744(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
enc2:
		/encm	a,d,c,-30
		xor	-30720(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-29
		xor	-29696(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-28
		xor	-28672(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-27
		xor	-27648(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-26
		xor	-26624(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-25
		xor	-25600(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-24
		xor	-24576(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-23
		xor	-23552(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-22
		xor	-22528(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-21
		xor	-21504(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-20
		xor	-20480(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-19
		xor	-19456(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-18
		xor	-18432(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-17
		xor	-17408(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-16
		xor	-16384(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-15
		xor	-15360(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-14
		xor	-14336(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-13
		xor	-13312(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-12
		xor	-12288(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-11
		xor	-11264(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-10
		xor	-10240(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-9
		xor	-9216(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-8
		xor	-8192(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-7
		xor	-7168(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-6
		xor	-6144(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-5
		xor	-5120(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-4
		xor	-4096(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-3
		xor	-3072(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		mov	%edx,-2048(%edi)	/store remainder bytes
		mov	%ecx,-1024(%edi)
		mov	%eax,(%edi)
		add	$4,%edi
		decw	-4(%ebp)		/loop till done
		jnz	enc0
		pop	%esi			/exit
		pop	%edi
		pop	%ebx
		leave
		ret

/********************************************************/
/	xl_chk(bfr, nblks)   3/32  1024 byte blocks	*/
/	out:	eax = 0 if ok				*/
/********************************************************/
		.globl	xl_chk
xl_chk:
		enter	$0, $4		/ loop count is a local variable
		push	%ebx
		push	%edi
		push	%esi
		mov	8(%ebp),%edi	/edi = adr
		mov	12(%ebp),%ebx	/ebx = # blocks
		mov	%ebx,%eax 	/adjust edi
		dec	%eax
		shl	$10,%eax
		add	%eax,%edi
		mov	$32,%eax		/set chkj according to # blocks
		subb	%bl,%al
		mulb	xl_chks
		add	$chk1,%eax
		mov	%eax,chkj
		mov	$256,-4(%ebp) 	/set up for check loop
chk0:
		mov	$xl_mxc0,%esi
		xor	%ebx,%ebx
		xor	%edx,%edx 	/edx ends up = r2
		xor	%ecx,%ecx 	/ecx ends up = r1
		xor	%eax,%eax 	/eax ends up = r0
		jmp	*chkj
/		.even
		.align	2
/
/		encode data
/
chk1:
		/encm	c,a,d,-31
		xor	-31744(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
chk2:
		/encm	a,d,c,-30
		xor	-30720(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-29
		xor	-29696(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-28
		xor	-28672(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-27
		xor	-27648(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-26
		xor	-26624(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-25
		xor	-25600(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-24
		xor	-24576(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-23
		xor	-23552(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-22
		xor	-22528(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-21
		xor	-21504(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-20
		xor	-20480(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-19
		xor	-19456(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-18
		xor	-18432(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-17
		xor	-17408(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-16
		xor	-16384(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-15
		xor	-15360(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-14
		xor	-14336(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-13
		xor	-13312(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-12
		xor	-12288(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-11
		xor	-11264(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-10
		xor	-10240(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-9
		xor	-9216(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-8
		xor	-8192(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-7
		xor	-7168(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-6
		xor	-6144(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		/encm	d,c,a,-5
		xor	-5120(%edi),%edx	/eax = new r0 bytes
		xor	%ecx,%eax		/set up edx for later
		movb	%dl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%cl		/ and xor to ecx
		movb	%dh,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		movb	%dl,%bl
		xorb	(%ebx,%esi),%cl
		movb	%dh,%bl
		xorb	(%ebx,%esi),%ch
		rol	$16,%edx
		rol	$16,%ecx
		xor	%ecx,%eax		/edx = new r1
		/encm	c,a,d,-4
		xor	-4096(%edi),%ecx	/eax = new r0 bytes
		xor	%eax,%edx		/set up edx for later
		movb	%cl,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%al		/ and xor to ecx
		movb	%ch,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		movb	%cl,%bl
		xorb	(%ebx,%esi),%al
		movb	%ch,%bl
		xorb	(%ebx,%esi),%ah
		rol	$16,%ecx
		rol	$16,%eax
		xor	%eax,%edx		/edx = new r1
		/encm	a,d,c,-3
		xor	-3072(%edi),%eax	/eax = new r0 bytes
		xor	%edx,%ecx		/set up edx for later
		movb	%al,%bl 		/multiply 4 bytes by c0
		xorb	(%ebx,%esi),%dl		/ and xor to ecx
		movb	%ah,%bl 		/ ecx will be new r2
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		movb	%al,%bl
		xorb	(%ebx,%esi),%dl
		movb	%ah,%bl
		xorb	(%ebx,%esi),%dh
		rol	$16,%eax
		rol	$16,%edx
		xor	%edx,%ecx		/edx = new r1
		xor	-2048(%edi),%edx	/check encoded data
		xor	-1024(%edi),%ecx
		xor	(%edi),%eax
		add	$4,%edi			/adv 4 columns
		or	%ecx,%eax 		/br if error
		or	%edx,%eax
		jnz	chk3
		decw	-4(%ebp)		/loop till done
		jnz	chk0
chk3:
		pop	%esi			/exit
		pop	%edi
		pop	%ebx
		leave
		ret
/********************************************************/
/	mpy	eax = ch*cl				*/
/********************************************************/
_mpy:
		movb	$0,%al
		orb	%cl,%cl		/return if 0
		je	mpy0
		orb	%ch,%ch
		je	mpy0
/
/	al=log[ch]+log[cl]
/
		xor	%ebx,%ebx
		movb	%ch,%bl
		movb	xl_log2(%ebx),%al

		movb	%cl,%bl
/
/	al = sum modulo 255
/
		addb	xl_log2(%ebx),%al
		adcb	$1,%al
		adcb	$255,%al

		mov	$xl_alg2,%ebx
		xlat
mpy0:
		ret

