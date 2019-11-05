CFLAGS=-O2 -Wall
CC=gcc

all: ADPCM_Crunch ADPCM_Decrunch

ADPCM_Decrunch: ADPCM_Decrunch.c
	$(CC) $(CFLAGS) -o ADPCM_Decrunch ADPCM_Decrunch.c

ADPCM_Crunch: ADPCM_Crunch.c
	$(CC) $(CFLAGS) -o ADPCM_Crunch ADPCM_Crunch.c
	
clean:
	rm ADPCM_Crunch ADPCM_Decrunch *.o
