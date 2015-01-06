/////////
/
/ Assigned and. If field, the right tree has been modified to have the
/ clearing mask ored into it. This means that an and may be aimed at the
/ word or byte containing the field and all bits not actually in the field
/ will be unchanged.
/
/////////

AAND:
%	PEFFECT|PRVALUE|PREL
	WORD		ANYR	*	*	TEMP
		RREG|MMX	WORD
		EASY|MMX	WORD
%	PEFFECT|PRVALUE|PREL
	WORD		ANYR	*	*	TEMP
		ADR|LV		WORD
		IMM|MMX		WORD
			[ZAND]	[AL],[AR]
		[IFV]	[ZMOV]	[R],[AL]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PREL
	WORD		AX	*	*	AX
		ADR|LV		FS8
		IMM|MMX		WORD
			[ZANDB]	[AL],[AR]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZCBW]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PREL
	WORD		AX	*	*	AX
		ADR|LV		FU8
		IMM|MMX		WORD
			[ZANDB]	[AL],[AR]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZSUBB]	[HI R],[HI R]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PREL|P_SRT
	WORD		ANYR	*	ANYR	TEMP
		ADR|LV		WORD
		TREG		WORD
			[ZAND]	[AL],[R]
		[IFV]	[ZMOV]	[R],[AL]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PREL|P_SRT
	WORD		AX	*	AX	AX
		ADR|LV		FS8
		TREG		WORD
			[ZANDB]	[AL],[LO R]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZCBW]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PREL|P_SRT
	WORD		AX	*	AX	AX
		ADR|LV		FU8
		TREG		WORD
			[ZANDB]	[AL],[LO R]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZSUBB]	[HI R],[HI R]
		[IFR]	[REL0]	[LAB]

%	PEFFECT|PRVALUE|PLT|PGE
	LONG		DXAX	*	*	DXAX
		ADR|LV		LONG
		LHC|MMX		LONG
			[ZMOV]	[LO AL],[LO AR]
			[ZAND]	[HI AL],[HI AR]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]
		[IFR]	[REL1]	[LAB]

%	PEFFECT|PRVALUE
	LONG		DXAX	*	*	DXAX
		ADR|LV		LONG
		LHS|MMX		LONG
			[ZAND]	[HI AL],[HI AR]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]

%	PEFFECT|PRVALUE
	LONG		DXAX	*	*	DXAX
		ADR|LV		LONG
		UHC|MMX		LONG
			[ZAND]	[LO AL],[LO AR]
			[ZMOV]	[HI AL],[HI AR]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]

%	PEFFECT|PRVALUE
	LONG		DXAX	*	*	DXAX
		ADR|LV		LONG
		UHS|MMX		LONG
			[ZAND]	[LO AL],[LO AR]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]

%	PEFFECT|PRVALUE|PLT|PGE
	LONG		DXAX	*	*	DXAX
		ADR|LV		LONG
		IMM|MMX		LONG
			[ZAND]	[LO AL],[LO AR]
			[ZAND]	[HI AL],[HI AR]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]
		[IFR]	[REL1]	[LAB]

%	PEFFECT|PRVALUE
	LONG		DXAX	*	DXAX	DXAX
		ADR|LV		LONG
		TREG		LONG
			[ZAND]	[LO AL],[LO R]
			[ZAND]	[HI AL],[HI R]
		[IFV]	[ZMOV]	[LO R],[LO AL]
		[IFV]	[ZMOV]	[HI R],[HI AL]

/////////
/
/ Fields. Distinguish between signed and unsigned fields. If the value
/ is needed the mask must be done for unsigned fields. The sign extend
/ shifts don't need the mask, so it gets optimized out.
/
/////////

%	PEFFECT|PRVALUE
	FS16		ANYR	*	*	TEMP
		ADR|LV		FFLD16
		DIR|IMM|MMX	WORD
			[ZAND]	[AL],[AR]
		[IFV]	[ZMOV]	[R],[AL]

%	PEFFECT|PRVALUE
	UWORD		ANYR	*	*	TEMP
		ADR|LV		FFLD16
		DIR|IMM|MMX	WORD
			[ZAND]	[AL],[AR]
		[IFV]	[ZMOV]	[R],[AL]
		[IFV]	[ZAND]	[R],[LO EMASK]

%	PEFFECT|PRVALUE
	FS16		AX	*	*	TEMP
		ADR|LV		FFLD8
		DIR|IMM|MMX	WORD
			[ZANDB]	[AL],[AR]
		[IFV]	[ZMOVB]	[LO R],[AL]

%	PEFFECT|PRVALUE
	UWORD		ANYR	*	*	TEMP
		ADR|LV		FFLD8
		DIR|IMM|MMX	WORD
			[ZANDB]	[AL],[AR]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZAND]	[R],[LO EMASK]

%	PEFFECT|PRVALUE|P_SRT
	FS16		ANYR	*	ANYR	TEMP
		ADR|LV		FFLD16
		TREG		WORD
			[ZAND]	[AL],[R]
		[IFV]	[ZMOV]	[R],[AL]

%	PEFFECT|PRVALUE|P_SRT
	UWORD		ANYR	*	ANYR	TEMP
		ADR|LV		FFLD16
		TREG		WORD
			[ZAND]	[AL],[R]
		[IFV]	[ZMOV]	[R],[AL]
		[IFV]	[ZAND]	[R],[LO EMASK]

%	PEFFECT|PRVALUE|P_SRT
	FS16		AX	*	AX	TEMP
		ADR|LV		FFLD8
		TREG		WORD
			[ZANDB]	[AL],[LO R]
		[IFV]	[ZMOVB]	[LO R],[AL]

%	PEFFECT|PRVALUE|P_SRT
	UWORD		AX	*	AX	TEMP
		ADR|LV		FFLD8
		TREG		WORD
			[ZANDB]	[AL],[LO R]
		[IFV]	[ZMOVB]	[LO R],[AL]
		[IFV]	[ZAND]	[R],[LO EMASK]
