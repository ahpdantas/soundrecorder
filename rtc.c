/*
 * rtc.c
 *
 *  Created on: 26 de ago de 2016
 *      Author: andre
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "utils/ustdlib.h"
#include "fatfs/ff.h"

#define FORMATTED_TIME_BUFFER_SIZE 32


volatile uint8_t second;
volatile uint8_t minute;
volatile uint8_t hour;
volatile uint32_t day;
volatile uint8_t systicks;
volatile uint32_t clockRate;
volatile bool displayTime = false;
char formattedTime[FORMATTED_TIME_BUFFER_SIZE];

void RTCIntHandler()
{
	systicks++;

	if(systicks == 4)
	{

		systicks = 0;

		if(second < 59)
		{
			second++;
			if(displayTime)
			{
				usnprintf(formattedTime,FORMATTED_TIME_BUFFER_SIZE,"%.2u:%.2u:%.2u",hour,minute,second);
			}

		}else
		{
			second = 0;
			minute++;
			if(displayTime)
			{
				usnprintf(formattedTime,FORMATTED_TIME_BUFFER_SIZE,"%.2u:%.2u:%.2u",hour,minute,second);
			}
		}

		if(minute == 60)
		{
			minute = 0;
			hour++;
		}

		if(hour == 24)
		{
			hour = 0;
			day++;
		}

	}
}

DWORD get_fattime (void)
{

    return    ((2014UL-1980) << 25)    // Year = 2007
            | (6UL << 21)            // Month = June
            | ((day+(1UL)) << 16)            // Day = 5
            | (hour << 11)            // Hour = 11
            | (minute << 5)            // Min = 38
            | (second >> 1)                // Sec = 0
            ;

}
