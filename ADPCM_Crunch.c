#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adpcm.h"

FILE *adpcm;
FILE *inRAW;
BOOL Sign = TRUE;
BOOL SVX = FALSE;

BOOL OpenSource(STRPTR FromFile, ULONG *SourceStart, ULONG *SourceLen, ULONG *Freq, UBYTE *Stereo, BOOL *eightbit)
{
	ULONG ID;
	ULONG Len;
	
	BOOL Break  = FALSE;
	BOOL Success= FALSE;
	BOOL Scan   =  TRUE;
	BOOL Error  = FALSE;
	BOOL WAVE   = FALSE;
	
	if (!(inRAW = fopen((char *)FromFile, "rb")))
	{
		printf("Error opening source file '%s'\n",FromFile);
	}
	else
	{
		if (fread(&ID, 1, sizeof(ID), inRAW) == 4)
		{
			if (ID == MAKE_ID('F','O','R','M'))
			{
				fseek(inRAW, 4, SEEK_CUR);
				
				if (fread(&ID, 1, sizeof(ID), inRAW) == 4)
				{
					if (ID == MAKE_ID('8','S','V','X'))
					{
						struct Voice8Header vheader;
						LONG twochan = 0;

						SVX = TRUE;
						Sign = FALSE;
						*eightbit = TRUE;
						
						while(Scan && (!Error) && (!Break))
						{
							if (fread(&ID, 1, sizeof(ID), inRAW) != 4)
							{
								Error = TRUE; break;
							}
							if (fread(&Len, 1, sizeof(Len), inRAW) != 4)
							{
								Error = TRUE; break;
							} else {
								Len = SWAP(Len);
							}
							
							switch(ID)
							{
								case MAKE_ID('V','H','D','R'):
									printf("Found VHDR chunk size %d\n", Len);
									if ( fread(&vheader, 1, sizeof(vheader), inRAW) != sizeof(vheader))
									{
										Error = TRUE;
										break;
									}
									else
									{
										*Freq = SWAPW(vheader.samplesPerSec);
										
										fseek(inRAW, Len-sizeof(vheader), SEEK_CUR);
									}
									break;

								case MAKE_ID('C', 'H', 'A', 'N'):
									printf("Found CHAN chunk size %d\n", Len);
									if( fread(&twochan, 1, Len, inRAW) != Len)
									{
										Error = TRUE;
										break;
									} else if (twochan == SWAP(6L)) {
										*Stereo = 2;
									}
									break;

								case MAKE_ID('B','O','D','Y'):
									printf("Found BODY chunk size %d\n",Len);
									*SourceLen = Len;
									Success = TRUE;
									Scan    = FALSE;
									break;
								
								default:
#ifdef __AMIGA__
									// printf("Found %lc%lc%lc%lc chunk size %d\n",
									// 	(ULONG)((ID>>24)&0xff),
									// 	(ULONG)((ID>>16)&0xff),
									// 	(ULONG)((ID>> 8)&0xff),
									// 	(ULONG)((ID    )&0xff),
									// 	Len);
#else
									printf("Found %lc%lc%lc%lc chunk size %d\n",
										(int)((ID    )&0xff),
										(int)((ID>> 8)&0xff),
										(int)((ID>>16)&0xff),
										(int)((ID>>24)&0xff),
										Len);
#endif
									fseek(inRAW, Len, SEEK_CUR);
									break;
							}
						}
						
						if (Error)
						{
							printf("Error, cannot handle this IFF 8SVX file!\n");
						}
					}
				}
			} else if(ID == MAKE_ID('R','I','F','F')) {
				fseek(inRAW, 4, SEEK_CUR);
				
				if (fread(&ID, 1, sizeof(ID), inRAW) == 4)
				{
					if (ID == MAKE_ID('W','A','V','E'))
					{
						struct WAVHeaderFmt wfheader;

						WAVE = TRUE;
						
						while(Scan && (!Error) && (!Break))
						{
							if (fread(&ID, 1, sizeof(ID), inRAW) != 4)
							{
								Error = TRUE; break;
							}
							if (fread(&Len, 1, sizeof(Len), inRAW) != 4)
							{
								Error = TRUE; break;
							}
							
							switch(ID)
							{
								case MAKE_ID('f','m','t',' '):
									printf("Found 'fmt ' chunk size %d\n", Len);
									if ( fread(&wfheader, 1, Len, inRAW) != Len)
									{
										Error = TRUE;
										break;
									}
									else
									{
										*Freq = wfheader.SamplesPerSec;

										if(wfheader.bitsPerSample > 16) {
											Error = TRUE;
											printf("Only 8 and 16bit WAVE files are support!\n");
											break;
										} else if(wfheader.bitsPerSample == 16) {
											*eightbit = FALSE;
											Sign = FALSE;
										}
										if(wfheader.NumOfChan == 2) {
											*Stereo = 2;
										} else if(wfheader.NumOfChan > 2) {
											Error = TRUE;
											printf("More than 2 channel WAVE files are not supported!\n");
											break;
										}
										fseek(inRAW, Len-sizeof(wfheader), SEEK_CUR);
									}
									break;
								
								case MAKE_ID('d','a','t','a'):
									printf("Found data chunk size %d\n",Len);
									*SourceLen = Len;
									Success = TRUE;
									Scan    = FALSE;
									break;
								default:
#ifdef __AMIGA__
									// printf("Found %lc%lc%lc%lc chunk size %d\n",
									// 	(ULONG)((ID>>24)&0xff),
									// 	(ULONG)((ID>>16)&0xff),
									// 	(ULONG)((ID>> 8)&0xff),
									// 	(ULONG)((ID    )&0xff),
									// 	Len);
#else
									printf("Found %lc%lc%lc%lc chunk size %d\n",
										(int)((ID    )&0xff),
										(int)((ID>> 8)&0xff),
										(int)((ID>>16)&0xff),
										(int)((ID>>24)&0xff),
										Len);
#endif
									fseek(inRAW, Len, SEEK_CUR);
									break;
							}
						}
						
						if (Error)
						{
							printf("Error, cannot handle this IFF 8SVX file!\n");
						}
					}
				}
			}
		}
		
		if ((!SVX) && (!WAVE) && (!Break) && (!Error))
		{
			fseek(inRAW, 0, SEEK_END);
			*SourceLen = ftell(inRAW);
			fseek(inRAW, 0, SEEK_SET);

			Success = TRUE;
		}
		
		if (Success && (!Break) && (!Error))
		{
			*SourceStart = ftell(inRAW);
		}
		
		if (Break || Error) Success = FALSE;
	}
	return(Success);
}

ULONG CompressADPCM(UBYTE Bits, UBYTE *Source, ULONG Length, UBYTE *Destination, ULONG JoinCode)
{
	WORD EstMax = (WORD)(JoinCode & 0xffff);
	UWORD Delta = (UWORD)((JoinCode & 0xffff0000) >> 16);
	ULONG lDelta = 0;
	UBYTE Samples = 24/Bits;

	if(!Delta) Delta = 5;

	Length = (Length+Samples-1)/Samples;

	while(Length--) {
		UBYTE sampleCount = 24/Bits;

		union Shift {
			ULONG inlong;
			UBYTE inbytes[4];
		} Shifter;

		Shifter.inlong = 0;

		while(sampleCount--) {
			WORD newDelta = (((BYTE)*Source++) << 6);
			WORD newEstMax = Delta >> 1;
			UBYTE nMax = (1<<(Bits - 1));

			Shifter.inlong <<= Bits;

			newDelta -= EstMax;

			if(newDelta < 0) {
				Shifter.inbytes[0] |= (1 << (Bits - 1)); // (1 << (3-1)) = %100 = 0x4;
				newDelta = -newDelta;
			}

			while (--nMax && newDelta > Delta) {
				newDelta -= Delta;
				Shifter.inbytes[0]++;
			}
			if (Bits == 4 && ((Shifter.inbytes[0] & 0x0f) == 0)) {
				Shifter.inbytes[0] &= 0xf0;
				Shifter.inbytes[0] |= 8;
			}

			UBYTE b = (Shifter.inbytes[0]& bitmask[Bits-1]);

			while(b--) {
				newEstMax += Delta;
			}

			lDelta = Delta * Matrix[Bits-2][Shifter.inbytes[0]& bitmask[Bits-1]];

			if(Shifter.inbytes[0] & (1 << (Bits - 1))) {	// SignBit
				newEstMax = -newEstMax;
			}
			EstMax = (EstMax + newEstMax) & 0xffff;
			
			Delta = (UWORD)((LONG)(lDelta + 8192) >> 14);
			
			newEstMax = EstMax >> 6;
			if(Delta < 5) Delta = 5;
		}
		*Destination++ = Shifter.inbytes[2];
		*Destination++ = Shifter.inbytes[1];
		*Destination++ = Shifter.inbytes[0];
	}
	return (Delta<<16|(EstMax&0xffff));
}

ULONG getfilesize(FILE *file) {
	ULONG size;
	
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	rewind(file);
	return size;
}

void convertSigned(UBYTE *buffer, ULONG size) {
	UBYTE *destbuff = buffer;

	while(size--) {
		*destbuff++ -= 128;
	}
}

void stereo2mono(UBYTE *buffer, ULONG size) {
	BYTE *destbuf = (BYTE *)buffer;
	BYTE *src  = (BYTE *)buffer;

	while(size-=2) {
		*destbuf++ = (src[0] >> 1) + (src[1] >> 1);
		src += 2;
	}
}

void stereo2mono8svx(UBYTE *buffer, ULONG size) {
	BYTE *destbuf = (BYTE *)buffer;
	BYTE *src_right  = (BYTE *)buffer;
	BYTE *src_left = (BYTE *)(buffer + size/2);

	while(size-=2) {
		*destbuf++ = (src_left[0] >> 1) + (src_right[0] >> 1);
		src_right += 1;	src_left += 1;
	}
}

void sixteen2eight(BYTE *buffer, ULONG size) {
	BYTE *destbuf = buffer;
	BYTE *src  = buffer;
	LONG sample = 0;

	while(size-=2) {
		sample = src[1];
		if (sample >  127) sample=  127;
		if (sample < -128) sample= -128;

		*destbuf++ = sample;
		src += 2;
	}

}
void Usage(STRPTR prog) {
	printf("Usage: %s [-f<frequency>] [-b<bits>] <infile> <outfile>\n", prog);
	printf("\t-f  frequency of a raw pcm file\n");
	printf("\t-b  bit size of target adpcm file (default 2)\n");
}


int main(int argc, char **argv) {
	ULONG destsize = 0, DataStart, DataLen;
	ULONG Frequency = 22050;
	ULONG Buffers = 64 * 1024;
	UBYTE hString[6];
	UBYTE Bits = 2, Stereo = 0;
	UBYTE *src = 0, *dest = 0;
	BOOL eightbit = TRUE;

	if(argc > 2) {
		STRPTR infile = argv[1], outfile = argv[2];
		UBYTE args = 1;

		while(argc--) {
			if(argv[args][0] == '-') {
				switch(argv[args][1]) {
					case 'b':
						Bits = atoi(&argv[args][2]);
						break;
					case 'f':
						Frequency = atol(&argv[args][2]);
						break;
					default:
						printf("Unknown parameter -%c !\n", argv[args][1]);
						Usage(argv[0]);
						goto end;
						break;
				}
			} else {
				infile = argv[args];
				outfile = argv[args+1];
				break;
			}
			args++;
		}

		printf("Bits: %d, %s to %s\n", Bits, infile, outfile);

		if(!OpenSource(infile, &DataStart, &DataLen, &Frequency, &Stereo, &eightbit)) {
			printf("Couldn't open infile %s\n", infile);
			goto end;
		}

		printf("DataStart: %d, DataLen: %d, Frequency: %d\n", DataStart,DataLen, Frequency);

		fseek(inRAW, DataStart, SEEK_SET);
		src = malloc(DataLen);
		if(src) {
			fread(src, 1, DataLen, inRAW);

			if(eightbit == FALSE) {
				printf("16bit source, converting to 8bit...\n");
				sixteen2eight((BYTE *)src, DataLen);
				DataLen /= 2;
			}

			if(Sign == TRUE) {				// WAVE file data is unsigned
				convertSigned(src, DataLen);
			}

			if(Stereo == 2) {
				printf("Stereo source, converting to mono...\n");

				if(SVX == TRUE) {
					stereo2mono8svx(src, DataLen);
				} else {
					stereo2mono(src,DataLen);
				}
				DataLen /= 2;
			}
		} else {
			printf("Couldn't allocate source buffer!\n");
			goto end;
		}

		adpcm = fopen(outfile, "wb");
		if(!adpcm) {
			printf("Couldn't open outfile %s\n", outfile);
			goto end;
		}

		if(Bits == 2) destsize = ((DataLen+3)/4);
		if(Bits == 3) destsize = ((DataLen+7)/8*3);
		if(Bits == 4) destsize = ((DataLen+2)/2);

		dest = malloc(destsize);
		if(dest) {
			sprintf((char *)hString, "ADPCM%d", Bits);
			fwrite(hString, 1, 6, adpcm);

			Frequency = SWAP(Frequency);
			fwrite(&Frequency, 1, 4, adpcm);

			CompressADPCM(Bits, src, DataLen, dest, 0);

			fwrite(dest, 1, destsize, adpcm);
		} else {
			printf("Couldn't allocate memory for destination buffer!\n");
		}

end:
		if(dest) free(dest);
		if(src)  free(src);
		if(adpcm) fclose(adpcm);
		if(inRAW) fclose(inRAW);

		return 0;
	}

	return 0;
}