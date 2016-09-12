/*
 * fallsensor.h
 *
 *  Created on: 6 de set de 2016
 *      Author: andre
 */

#ifndef WAVEREC_H_
#define WAVEREC_H_

#include "config.h"
#include "../wave.h"

typedef enum
{
	CREATE_WAVE,
	ADD_SAMPLES,
	END_WAVE
}WAVE_STATES_DEF;

typedef struct{
	struct{
		unsigned short _0[WAVE_BUFFER_SIZE];
		unsigned short _1[WAVE_BUFFER_SIZE];
		unsigned short *active;
		unsigned short *toSave;
		unsigned int size;
	}buff;
	struct{
		unsigned int Active:1;
		unsigned int IsReadyToSave:1;
		unsigned int EndRecord:1;
	}flgs;
	WAVE Wave;
	WAVE_STATES_DEF States;
}WAVE_REC_DEF;


#endif /* FALLSENSOR_H_ */
