/*
 * wave.h
 *
 *  Created on: 8 de set de 2016
 *      Author: andre
 */

#ifndef WAVE_H_
#define WAVE_H_

typedef struct WAVE_DEF* WAVE;

void CreateWave(WAVE* Wave,unsigned short sampleRate, unsigned short numChannels, unsigned short bitsPerSample);
int AddSample(WAVE Wave, unsigned short *samples, unsigned int samplesBuffersize, unsigned int numberOfSamples );
void EndWave(WAVE Wave);

#endif /* WAVE_H_ */
