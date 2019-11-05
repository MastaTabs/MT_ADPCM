
	***  ADPCM3 sample decompression routines   ****
	*** Copyright (C) 1993 by Christian Buchner ****


		SECTION	Code


	*** DecompressADPCM3 ***

	; JoinCode = DecompressADPCM3(Source, Length, Destination, JoinCode)
	; d0                          a0      d0      a1           d1
	;
	; Length _MUST_ be a multiple of 3!
	;
	; This function decompresses 3bit ADPCM data to a given memory. The
	; result is an 8 bit raw sample which can be played by the Amiga
	; audio DMA. The destination buffer must be at least 8/3 times as
	; large as the source buffer.
	;
  ; The JoinCode is used to decompress a sample in parts. Just hand in
	; the return value of the last decompressed chunk and continue at the
	; position you stopped. The JoinCode should be set to 0 when starting
	; a new decompression.
	; 
	; Bits 31-16      Bits 15-0
	; Current Delta | Current EstMax
	;

		XDEF _DecompressADPCM3
_DecompressADPCM3
		movem.l	d2-d4,-(sp)

		move.w	d1,d3			; d3=EstMax
		swap	d1
		move.w	d1,d2			; d2=Delta
		bne.s	d3_loop
		moveq	#5,d2

d3_loop		move.b	(a0)+,d1		; d1=Shifter
		rol.b	#3,d1
		bsr.s	d3_byte
		rol.b	#3,d1
		bsr.s	d3_byte
		lsl.w	#2,d1
		move.b	(a0)+,d1
		ror.w	#7,d1
		bsr.s	d3_byte
		rol.w	#3,d1
		bsr.s	d3_byte
		rol.w	#3,d1
		bsr.s	d3_byte
		lsr.w	#7,d1
		move.b	(a0)+,d1
		ror.w	#6,d1
		bsr.s	d3_byte
		rol.w	#3,d1
		bsr.s	d3_byte
		rol.w	#3,d1
		bsr.s	d3_byte

d3_entry	subq.l	#3,d0			; d0=Counter
		bhi.s	d3_loop

		move.w	d2,d0			; -> d0=JoinCode
		swap	d0
		move.w	d3,d0

		movem.l	(sp)+,d2-d4
		rts

d3_byte		bsr	adaptive
		move.w	d3,d4
		asr.w	#6,d4
		cmp.w	#127,d4
		bgt.s	d3_high
		cmp.w	#-128,d4
		blt.s	d3_low
		move.b	d4,(a1)+
		rts
d3_high		move.b	#127,(a1)+
		rts
d3_low		move.b	#-128,(a1)+
		rts



		*** Adaptions-Routine ***

adaptive	; d1 = SignBit + DataBit

		move.w	d2,d4		//d2 Delta
		lsr.w	#1,d4
		btst	#1,d1
		beq.s	d3_0
d3_1		btst	#0,d1
		beq.s	d3_10
d3_11		add.w	d2,d4
		add.w	d2,d4
		add.w	d2,d4
		mulu	#$6607,d2
		bra.s	d3_sign
d3_10		add.w	d2,d4
		add.w	d2,d4
		mulu	#$4D14,d2
		bra.s	d3_sign
d3_0		btst	#0,d1
		beq.s	d3_00
d3_01		add.w	d2,d4
		mulu	#$3A9F,d2
		bra.s	d3_sign
d3_00		mulu	#$399A,d2
d3_sign		btst	#2,d1
		beq.s	d3_add
		neg.w	d4
d3_add		add.w	d4,d3
		add.l	#8192,d2
		moveq	#14,d4
		asr.l	d4,d2
		rts


		END
