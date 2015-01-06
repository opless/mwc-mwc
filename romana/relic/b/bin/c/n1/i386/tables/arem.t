//////////
/ n1/i386/tables/arem.t
//////////

/////////
/
/ Assigned remainder (%=).
/ Just like /=, except the result is in EDX instead of EAX.
/ Use unsigned divide if either left or right operand is unsigned.
/ No doubles, since the operation is not defined on doubles.
/
/////////

AREM:

/ Unsigned integer by unsigned.
/ The result word or byte does not have to be sign- or zero-extended
/ in value context, the result is already correct.
%	PEFFECT|PVALUE
	LONG		EDXEAX	*	*	EDX
		ADR|LV		UINT
		ADR		LONG
/ Signed long by unsigned.
%	PEFFECT|PVALUE
	LONG		EDXEAX	*	*	EDX
		ADR|LV		FS32
		ADR		FU32
			[TL ZMOVSX]	[REGNO EAX], [AL]
			[ZSUB]		[REGNO EDX], [REGNO EDX]
			[ZDIV]		[AR]
			[TL ZMOV]	[AL], [TL REGNO EDX]

/ Signed word or byte by unsigned.
/ The result must be sign- or zero-extended in value context.
%	PEFFECT|PVALUE
	LONG		EDXEAX	*	*	EDX
		ADR|LV		FS16|FS8
		ADR		FU32
			[TL ZMOVSX]	[REGNO EAX], [AL]
			[ZSUB]		[REGNO EDX], [REGNO EDX]
			[ZDIV]		[AR]
			[TL ZMOV]	[AL], [TL REGNO EDX]
		[IFV]	[TL ZMOVSX]	[REGNO EDX], [TL REGNO EDX]

/ Signed integer by signed.
/ The result word or byte does not have to be sign- or zero-extended
/ in value context, the result is already correct.
%	PEFFECT|PVALUE
	LONG		EDXEAX	*	*	EDX
		ADR|LV		SINT
		ADR		FS32
			[TL ZMOVSX]	[REGNO EAX], [AL]
			[ZCDQ]
			[ZIDIV]		[AR]
			[TL ZMOV]	[AL], [TL REGNO EDX]

//////////
/ Fields.
/ The right hand operand of the field is not preshifted
/ like it is for add, subtract and the increment/decrement ops. 
//////////

/ Field.
%	PEFFECT|PVALUE
	LONG		EDXEAX	*	*	EDX
		ADR|LV		FLD
		ADR		LONG
			[TL ZMOVZX]	[REGNO EAX],[AL]	/ fetch
			[ZAND]		[REGNO EAX],[EMASK]	/ extract
			[ZSUB]		[REGNO EDX],[REGNO EDX]	/ widen
			[ZDIV]		[AR]			/ divide
			[ZAND]		[REGNO EAX],[EMASK]	/ mask
			[TL ZAND]	[AL],[TL CMASK]		/ clear
			[TL ZOR]	[AL],[TL REGNO EDX]	/ store

//////////
/ end of n1/i386/tables/arem.t
//////////
