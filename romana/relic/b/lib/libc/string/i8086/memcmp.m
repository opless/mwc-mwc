//////////
/ i8086 C string library.
/ memcmp()
/ ANSI 4.11.4.1.
//////////

//////////
/ memcmp(String1, String2, Count)
/ char *String1, *String2;
/ int Count;
/
/ Compare Count bytes of String2 and String1.
//////////

#include <larges.h>

String1	=	LEFTARG
String2	=	String1+DPL
Count	=	String2+DPL

	Enter(memcmp_)
	mov	cx, Count(bp)	/ Count to CX
	sub	ax, ax		/ Result 0 to AX, set Zero flag in case CX==0
	Lds	si, String2(bp)	/ String2 address to DS:SI
	Les	di, String1(bp)	/ String1 address to ES:DI
	cld
	repe
	cmpsb
	je	2f		/ String1 == String2
	ja	1f
	inc	ax		/ String1 > String2
	jmp	2f

1:	dec	ax		/ String1 < String2

2:	Leave
