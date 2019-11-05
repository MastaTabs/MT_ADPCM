#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adpcm.h"

ULONG DecompressADPCM(UBYTE Bits, UBYTE *Source, ULONG Length, UBYTE *Destination, ULONG JoinCode)
{
	WORD EstMax = (WORD)(JoinCode & 0xffff);
	UWORD Delta = (UWORD)((JoinCode & 0xffff0000) >> 16);
	ULONG lDelta = 0;
	
	if(!Delta) Delta = 5;
	
	Length /= 3;

	while(Length--) {
		UBYTE sampleCount = 24/Bits;
		ULONG temp = (Source[0] << 16) | (Source[1] << 8) | Source[2];
		Source+=3;

		while(sampleCount--) {
			WORD newEstMax = (Delta >> 1);
			UBYTE Shifter  = (temp >> sampleCount*Bits);
			UBYTE b = (Shifter & bitmask[Bits-1]);

			if ((Bits == 4) && ((Shifter & 0xf) == 0))
				Delta = 4;

			while(b--) {
				newEstMax += Delta;
			}

			lDelta = Delta * Matrix[Bits-2][Shifter & bitmask[Bits-1]];

			if(Shifter & (1<<(Bits-1))) {	// SignBit
				newEstMax = -newEstMax;
			}
			EstMax = (EstMax + newEstMax) & 0xffff;
			
			Delta = (UWORD)((LONG)(lDelta + 8192) >> 14);
			
			if(Delta < 5) Delta = 5;

			newEstMax = EstMax >> 6;
			if(127 < newEstMax)
				*Destination++ = 127;
			else if( -128 > newEstMax) {
				*Destination++ = -128;
			}
			else
				*Destination++ = newEstMax;
		}
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

void convertUnsigned(UBYTE *buffer, ULONG size) {
	UBYTE *destbuff = buffer;

	while(size--) {
		*destbuff++ += 128;
	}
}

void Usage(STRPTR prog) {
	printf("Usage: %s [-w] <infile> <outfile>\n", prog);
	printf("\t-w  write WAVE file instead of IFF 8SVX\n");
}

int main(int argc, char **argv)
{
	FILE *adpcm  = 0;
	FILE *outraw = 0;
	
	ULONG filesize, bufsize, destsize;
	ULONG JoinCode = 0;
	ULONG freq;
	UBYTE *buffer = 0;
	UBYTE *dest   = 0;
	UBYTE Bits;
	BOOL WAVE = FALSE;

	struct ADPCMHeader header;

	printf("%s - MT_ADPCM tools V%s by MastaTabs (c)2013\n", argv[0], "1.0");

	if(argc > 2) {
		STRPTR infile = 0, outfile = 0;
		UBYTE args = 1;

		while(argc--) {
			if(argv[args][0] == '-') {
				switch(argv[args][1]) {
					case 'w':
						WAVE = TRUE;
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

		adpcm = fopen(infile, "rb");
		outraw = fopen(outfile, "wb");
		
		if(!adpcm) {
			printf("Couldn't open input file %s!\n\n", infile);
			Usage(argv[0]);
			goto end;
		}
			
		if(!outraw) {
			printf("Couldn't open output file %s!\n\n", outfile);
			Usage(argv[0]);
			goto end;
		}
			
		filesize = getfilesize(adpcm);
		bufsize = filesize-sizeof(header);
		buffer = malloc(filesize-sizeof(header));
		
		if(!buffer) {
			printf("Couldn't allocate memory for input buffer!\n");
			goto end;
		}
			
		fread(&header, 1, sizeof(header), adpcm);
		fread(buffer, 0x1, bufsize, adpcm);

		freq = SWAP(header.Frequency);

		printf("infile: %s, outfile: %s\n", infile, outfile);
		printf("%s freq: %u bits: %c\n", header.Identifier, freq, header.Identifier[5]);

		char temp[2] = { header.Identifier[5] };
		Bits = atoi(temp);

		switch(Bits) {
			case 2:
				destsize = bufsize*4;
				break;
			case 3:
				destsize = (bufsize/3)*8;
				break;
			case 4:
				destsize = (bufsize*2);
				break;
			default:
				printf("This bit size is not supported!\n");
				goto end;
		}

		dest = malloc(destsize);
		if(!dest) {
			printf("Couldn't allocate memory for output buffer!\n");
			goto end;
		}

		JoinCode = DecompressADPCM(Bits, buffer, bufsize, dest, JoinCode);

		if(WAVE == TRUE) {
			struct WAVHeader wheader;

			wheader = (struct WAVHeader) { 
				.RIFF = MAKE_ID('R', 'I', 'F', 'F'),
				.WAVE = MAKE_ID('W', 'A', 'V', 'E'),
				.fmt  = MAKE_ID('f', 'm', 't', ' '),
				.whf.NumOfChan = 1,
				.whf.AudioFormat = 1, //PCM
				.whf.bitsPerSample = 8,
				.whf.blockAlign = 1*8/8,
				.Subchunk1Size = 16,
				.Subchunk2ID = MAKE_ID('d', 'a', 't', 'a'),
				.ChunkSize = sizeof(wheader) + destsize -8,
				.whf.SamplesPerSec = freq,
				.whf.bytesPerSec = 1 * wheader.whf.SamplesPerSec * 8 / 8,
				.Subchunk2Size = destsize
			};
	
			convertUnsigned(dest, destsize);

			fwrite(&wheader, 1, sizeof(wheader), outraw);
		} else {
			struct SVXHeader sheader;

			sheader = (struct SVXHeader) {
				.FORM = MAKE_ID('F', 'O', 'R', 'M'),
				.SVX  = MAKE_ID('8', 'S', 'V', 'X'),
				.VHDR = MAKE_ID('V', 'H', 'D', 'R'),
				.ANNO = MAKE_ID('A', 'N', 'N', 'O'),
				.BODY = MAKE_ID('B', 'O', 'D', 'Y'),
				.fSize = SWAP(((destsize+sizeof(struct SVXHeader))-8)),
				.vSize = SWAP(sizeof(struct Voice8Header)),
				.bSize = SWAP(destsize),
				.aSize = SWAP(sizeof(sheader.aData)),
				.aData = "created by MT_ADPCM",
				.v8h.samplesPerSec = SWAPW(freq),
				.v8h.oneShotHiSamples = SWAP(destsize),
				.v8h.ctOctave = 1,
				.v8h.volume = SWAP(0x10000L)
			};

			fwrite(&sheader, 1, sizeof(sheader), outraw);
		}
		
		fwrite(dest, 1, destsize, outraw);

end:
		if(dest) free(dest);
		if(buffer) free(buffer);

		if(adpcm) fclose(adpcm);
		if(outraw) fclose(outraw);

	} else {
		printf("no file given\n");
		Usage(argv[0]);
		return 1;
	}

	return 0;
}
