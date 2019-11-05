#ifndef __MT_ADPCM__
#define __MT_ADPCM__

#ifndef __AMIGA__
#include <inttypes.h>
#include <arpa/inet.h>

typedef uint8_t  UBYTE;
typedef uint16_t UWORD;
typedef uint32_t ULONG;
typedef int8_t  BYTE;
typedef int16_t WORD;
typedef int32_t LONG;
typedef char* STRPTR;
typedef uint8_t BOOL;

#define TRUE 1
#define FALSE 0

#define SWAP(x) ((0xff & x)<<24|(0x0000ff00 & x) << 8|(0x00ff0000 & x) >> 8| (0xff000000 & x)>>24)
#define SWAPW(x) (((x & 0xff00) >> 8) | ((x & 0x00ff) << 8))
#define attrib(x) __attribute__ ((x)) 
#else

#include <exec/types.h>
#define SWAP(x) x
#define SWAPW(x) x
#define attrib(x)
#endif

#define MAKE_ID(a,b,c,d)  SWAP((  (a)<<24 |  (b)<<16 | (c)<<8 | (d) ))


static int Matrix[3][8] =
{
	{0x3800, 0x5600, 0, 0, 0, 0, 0, 0},
	{0x399a, 0x3a9f, 0x4d14, 0x6607, 0, 0, 0, 0},
	{0x3556, 0x3556, 0x399A, 0x3A9F, 0x4200, 0x4D14, 0x6607, 0x6607},
};

static int bitmask[9] =
{
	0, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff
};

struct ADPCMHeader
{
	UBYTE Identifier[6];
	ULONG Frequency;
} attrib(packed);

struct Voice8Header {
    ULONG oneShotHiSamples, /* # samples in the high octave 1-shot part */
          repeatHiSamples,  /* # samples in the high octave repeat part */
          samplesPerHiCycle;/* # samples/cycle in high octave, else 0   */
    UWORD samplesPerSec;    /* data sampling rate                       */
    UBYTE ctOctave,         /* # octaves of waveforms                   */
          sCompression;     /* data compression technique used          */
    LONG volume;           /* playback volume from 0 to Unity (full
                             * volume). Map this value into the output
                             * hardware's dynamic range.                */
} attrib(packed);

struct SVXHeader
{
	ULONG FORM;
	ULONG fSize;
	ULONG SVX, VHDR;
	ULONG vSize;
	struct Voice8Header v8h;
	ULONG ANNO;
	ULONG aSize;
	UBYTE aData[20];
	ULONG BODY;
	ULONG bSize;
} attrib(packed);

struct WAVHeaderFmt
{
	UWORD	AudioFormat;
	UWORD	NumOfChan;
	ULONG	SamplesPerSec;
	ULONG	bytesPerSec;
	UWORD	blockAlign;
	UWORD	bitsPerSample;
} attrib(packed);

struct WAVHeader
{
	ULONG	RIFF;
	ULONG	ChunkSize;
	ULONG	WAVE;
	ULONG	fmt;
	ULONG	Subchunk1Size;
	struct WAVHeaderFmt whf;
	ULONG	Subchunk2ID;
	ULONG	Subchunk2Size;
} attrib(packed); 

#endif /* __MT_ADPCM__ */
