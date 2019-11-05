
	***  ADPCM2 sample decompression routines   ****
	*** Copyright (C) 1993 by Christian Buchner ****


		SECTION	Code


	*** DecompressADPCM2 ***

	; JoinCode = DecompressADPCM2(Source, Length, Destination, JoinCode)
	; d0                          a0      d0      a1           d1
	;
	; This function decompresses 2bit ADPCM data to a given memory. The
	; result is an 8 bit raw sample which can be played by the Amiga
	; audio DMA. The destination buffer must be at least 4 times as large
	; as the source buffer.
	;
        ; The JoinCode is used to decompress a sample in parts. Just hand in
	; the return value of the last decompressed chunk and continue at the
	; position you stopped. The JoinCode should be set to 0 when starting
	; a new decompression.
	; 
	; Bits 31-16      Bits 15-0
	; Current Delta | Current EstMax
	;

		XDEF _DecompressADPCM2
_DecompressADPCM2
		movem.l	d2-d4,-(sp)

		move.w	d1,d3			; d3=EstMax
		swap	d1
		move.w	d1,d2			; d2=Delta
		bne.s	d2_entry
		moveq	#5,d2
		bra.s	d2_entry

d2_loop		move.b	(a0)+,d1		; d1=Shifter
		rol.b	#2,d1
		bsr.s	d2_byte
		rol.b	#2,d1
		bsr.s	d2_byte
		rol.b	#2,d1
		bsr.s	d2_byte
		rol.b	#2,d1
		bsr.s	d2_byte

d2_entry	dbra	d0,d2_loop		; d0=Counter
		swap	d0
		subq.w	#1,d0
		bcs.s	d2_finished
		swap	d0
		bra.s	d2_loop

d2_finished	move.w	d2,d0			; -> d0=JoinCode
		swap	d0
		move.w	d3,d0

		movem.l	(sp)+,d2-d4
		rts

d2_byte		bsr.s	adaptive
		move.w	d3,d4
		asr.w	#6,d4
		cmp.w	#127,d4
		bgt.s	d2_high
		cmp.w	#-128,d4
		blt.s	d2_low
		move.b	d4,(a1)+
		rts
d2_high		move.b	#127,(a1)+
		rts
d2_low		move.b	#-128,(a1)+
		rts



		*** Adaptions-Routine ***

adaptive	; d1 = SignBit + DataBit

		move.w	d2,d4
		lsr.w	#1,d4
		btst	#0,d1
		beq.s	d2_skip1
		add.w	d2,d4
		mulu	#$5600,d2
		bra.s	d2_sign
d2_skip1	mulu	#$3800,d2
d2_sign		btst	#1,d1
		beq.s	d2_skip2
		neg.w	d4
d2_skip2	add.w	d4,d3
		add.l	#8192,d2
		moveq	#14,d4
		asr.l	d4,d2
		rts


		END
