Short:        De- & Cruncher for FP_ADPCM format (C source)\
Uploader:     tobi@themaster.de (Tobias Seiler)\
Author:       tobi@themaster.de (Tobias Seiler)\
Type:         util/pack\
Version:      1.0\
Architecture: generic\
Distribution: Aminet\
Kurz:         De- & Cruncher für das FP_ADPCM Format (C source)\
\
MT_ADPCM is a cruncher and decruncher for the FP_ADPCM format written in pure ANSI-C.

Some years ago I was looking for a package that I could port for AROS.\
I remembered the ADPCM package of Christian "FlowerPower" Buchner and started\
rewriting the ASM code in C.\
I didn't get very far. Means the resulting code didn't work.\
Over the years I've been coming back to it once in while and lately I made\
some progress quickly. Seems I had learned some things over the years ;).\
\
The code was developed on a 64bit Intel Linux system but compiles in VBCC.\
I haven't tested it on the Amiga yet. Time is a quite factor these days...\
\
It reads and writes source and destination files in a whole. So it needs\
quite a bit of memory. The actual crunch/decrunch functions are layed out\
like the ASM version. Using my functions instead should be low effort.\
\
The cruncher supports 8 and 16bit mono/stereo WAV files and 8SVX mono/stereo\
files as source. The destination format is 2, 3 or 4! bit ADPCM in mono.\
\
The decruncher supports 8bit WAV and 8SVX mono output.\
\
The 4bit format was implemented using some ideas of the mgetty unix
package.\
\
This is all Public Domain like the original!\
\
Enjoy!\
MastaTabs
