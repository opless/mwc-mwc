/ Hand-coded string functions for STREAMS implementation.

		.unixorder

		.globl	strlen

string		=	4

strlen:
		mov	%edi, %edx		/ Preserve %edi
		movl	string(%esp), %edi	/ String start

		movl	$-1, %ecx		/ Maximum string length
		xorl	%eax, %eax		/ Looking for 0
		cld				/ Scan upwards
		repne scasb			/ Find end-of-string

		movl	$-2, %eax		/ Inital %ecx - 1
		subl	%ecx, %eax		/ Get iteration count - 1

		movl	%edx, %edi		/ Restore %edi
		ret


/ String copy with length limit. An important part of the semantics of this
/ operation is the fact that the destination is null-padded out the maximum
/ length every time; the operating system depends on this for security at
/ some times.

		.globl	strncpy

dest		=	4
src		=	8
len		=	12

strncpy:
		movl	len(%esp), %ecx		/ Maximum length
		jecxz	?strncpy_done

		movl	%esi, %edx		/ Preserve %esi
		movl	src(%esp), %esi		/ Get src

		pushl	%edi			/ Preserve %edi
		movl	dest+4(%esp), %edi	/ Get dest, adjusting the
						/ stack offset for push

		cld				/ Scan upwards

?strncpy_loop:	lodsb				/ %al = * %ds:%esi ++
		stosb				/ * %es:%edi ++ = %al
		orb	%al, %al		/ %al == 0 ?
		loopne	?strncpy_loop

/ At some point it may be profitable to look at doing the zero-fill here in
/ 16-bit or 32-bit chunks.

		rep	stosb			/ Pad out destination, doing
						/ nothing if %ecx == 0

		popl	%edi			/ Restore %edi
		movl	%edx, %esi		/ Restore %esi

?strncpy_done:
		movl	dest(%esp), %eax	/ Return destination
		ret
