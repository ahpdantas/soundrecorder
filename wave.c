#include <stdio.h>
#include <string.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "utils/ustdlib.h"
#include "utils/uartstdio.h"
#include "wave.h"
#include "config.h"
#include "fatfs/ff.h"

#define WAVE_FILE_NAME_SIZE 16
#define WAVE_ID_MAX 9999

typedef struct {
    // Riff Wave Header
    char chunkId[4];
    int  chunkSize;
    char format[4];

    // Format Subchunk
    char subChunk1Id[4];
    int  subChunk1Size;
    short int audioFormat;
    short int numChannels;
    int sampleRate;
    int byteRate;
    short int blockAlign;
    short int bitsPerSample;
    //short int extraParamSize;

    // Data Subchunk
    char subChunk2Id[4];
    int  subChunk2Size;

} WAVE_HEADER_DEF;

typedef struct {
    WAVE_HEADER_DEF header;
    FIL File;
    CHAR name[WAVE_FILE_NAME_SIZE];
    long long int index;
    long long int size;
    long long int nSamples;
} WAVE_DEF;

static WAVE_DEF InternalWave;


void CreateWaveHeader(
		WAVE_HEADER_DEF *header,
		int const sampleRate,
		short int const numChannels,
		short int const bitsPerSample )
{
    // RIFF WAVE Header
    header->chunkId[0] = 'R';
    header->chunkId[1] = 'I';
    header->chunkId[2] = 'F';
    header->chunkId[3] = 'F';
    header->format[0] = 'W';
    header->format[1] = 'A';
    header->format[2] = 'V';
    header->format[3] = 'E';

    // Format subchunk
    header->subChunk1Id[0] = 'f';
    header->subChunk1Id[1] = 'm';
    header->subChunk1Id[2] = 't';
    header->subChunk1Id[3] = ' ';
    header->audioFormat = 1; // FOR PCM
    header->numChannels = numChannels; // 1 for MONO, 2 for stereo
    header->sampleRate = sampleRate; // ie 44100 hertz, cd quality audio
    header->bitsPerSample = bitsPerSample; //
    header->byteRate = header->sampleRate * header->numChannels * header->bitsPerSample / 8;
    header->blockAlign = header->numChannels * header->bitsPerSample/8;

    // Data subchunk
    header->subChunk2Id[0] = 'd';
    header->subChunk2Id[1] = 'a';
    header->subChunk2Id[2] = 't';
    header->subChunk2Id[3] = 'a';

    // All sizes for later:
    // chuckSize = 4 + (8 + subChunk1Size) + (8 + subChubk2Size)
    // subChunk1Size is constanst, i'm using 16 and staying with PCM
    // subChunk2Size = nSamples * nChannels * bitsPerSample/8
    // Whenever a sample is added:
    //    chunkSize += (nChannels * bitsPerSample/8)
    //    subChunk2Size += (nChannels * bitsPerSample/8)
    header->chunkSize = 4+8+16+8+0;
    header->subChunk1Size = 16;
    header->subChunk2Size = 0;

}

void GetWaveNameById(char* fileDst, unsigned int size, unsigned int Id )
{
	memset(fileDst,0,size);
	usnprintf(fileDst,size,"wave%04d.wav",Id);
}

void CreateNewWavename(char* fileDst, unsigned int size, UINT *Id )
{
	*Id += 1;
	if( *Id > WAVE_ID_MAX )
	{
		*Id = 0;
	}

	GetWaveNameById(fileDst,size,*Id);
}

int CreateWave( WAVE* Wave, unsigned short sampleRate, unsigned short numChannels, unsigned short bitsPerSample){
    static unsigned int Id = 0;
	FRESULT result;

    //CreateWaveHeader(&InternalWave.header,sampleRate,numChannels,bitsPerSample);
    CreateNewWavename(InternalWave.name,WAVE_FILE_NAME_SIZE, &Id);

    result = f_open( &InternalWave.File, InternalWave.name, FA_WRITE | FA_CREATE_ALWAYS );
 	if( result == FR_OK )
	{
 		*Wave = &InternalWave;
 		return 0;
	}
 	else
 	{
 		return -1;
 	}
}

int AddSample( WAVE Wave, unsigned short *samples, unsigned int samplesBuffersize, unsigned int numberOfSamples ){
    UINT bw = 0;
    FRESULT result;
    WAVE_DEF *w;
    
    if( Wave == NULL )
    {
    	return -1;
    }
    w = (WAVE_DEF*)Wave;
	result = f_write(&w->File, samples, samplesBuffersize, &bw );
    if( result == FR_OK )
	{
    	w->nSamples+= numberOfSamples;
    	return 0;
	}
    else
    {
    	GPIOPinWrite(GPIO_PORTF_BASE, RED_LED| BLUE_LED|GREEN_LED, RED_LED);
    	return -1;
    }

}

void EndWave(WAVE Wave)
{
    WAVE_DEF *w;

    if( Wave == NULL )
    {
    	return;
    }
    w = (WAVE_DEF *)Wave;

    if( f_close(&w->File) == FR_OK )
    {
    	UARTprintf("%s closed with success.\n", w->name);
    }


}
