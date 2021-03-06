#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"
#include "waverecorder.h"
#include "init.h"
#include "wave.h"

static WAVE_REC_DEF WaveRecorder;

unsigned int PacketsNotSaved = 0;
unsigned int PacketsSaved = 0;
unsigned int Packets = 0;

void Adc0_3_ISR(void)
{
	static unsigned int transfers = 0;
	static unsigned int Counter1Hz = 0;
	unsigned long AdcSample;
	// Clear the timer interrupt
	ADCIntClear(ADC0_BASE,3);

	ADCSequenceDataGet(ADC0_BASE, 3, &AdcSample );
	WaveRecorder.buff.toSave[transfers] = (unsigned int)AdcSample;

	transfers++;
	if( (transfers)%WAVE_BUFFER_SIZE == 0 )
	{
		Packets++;
		if( WaveRecorder.flgs.Active )
		{
			WaveRecorder.buff.toSave = WaveRecorder.buff._1;
			WaveRecorder.buff.active = WaveRecorder.buff._0;
		}
		else
		{
			WaveRecorder.buff.toSave = WaveRecorder.buff._0;
			WaveRecorder.buff.active = WaveRecorder.buff._1;
		}

		WaveRecorder.flgs.Active ^= 1;
		WaveRecorder.flgs.IsReadyToSave = 1;
		transfers = 0;
	}

	Counter1Hz++;
	if( Counter1Hz == SAMPLE_RATE )
	{
		Counter1Hz = 0;
		if(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) )
		{
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED| BLUE_LED|GREEN_LED, 0);
		}
		else
		{
			GPIOPinWrite(GPIO_PORTF_BASE, RED_LED| BLUE_LED|GREEN_LED, BLUE_LED);
		}
	}
}


void Timer1AIntHandler(void)
{
	// Clear the timer interrupt
	TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	WaveRecorder.flgs.EndRecord = 1;

}

void StartRecord()
{
	WaveRecorder.States = ADD_SAMPLES;
	TimerEnable(TIMER1_BASE,TIMER_A);
	UARTprintf("Adding samples...\n");
	Packets = 0;
}

void EndRecord()
{
	UARTprintf("Total:%d\n",Packets);
	WaveRecorder.flgs.EndRecord = 0;
	WaveRecorder.States = END_WAVE;
	TimerDisable(TIMER1_BASE,TIMER_A);
}

/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */

int main(void)
{
	init(&WaveRecorder);
	while(1)
   	{
		if( WaveRecorder.flgs.IsReadyToSave )
		{
			switch(WaveRecorder.States)
			{
			case CREATE_WAVE:
				CreateWave(&WaveRecorder.Wave,SAMPLE_RATE,1,16);
				StartRecord();
			case ADD_SAMPLES:
				if( AddSample(WaveRecorder.Wave,WaveRecorder.buff.toSave,WaveRecorder.buff.size, WAVE_BUFFER_SIZE) ){
					PacketsNotSaved++;
				}

				if( WaveRecorder.flgs.EndRecord )
				{
					UARTprintf("Packets Created:%d Packets Saved: %d. Packets Not Saved: %d - %d\n", Packets, PacketsSaved, PacketsNotSaved);
					PacketsNotSaved = 0;
					PacketsSaved = 0;
					EndRecord();
				}
				break;
			case END_WAVE:
				EndWave(WaveRecorder.Wave);
				WaveRecorder.States = CREATE_WAVE;
				break;
			}

			WaveRecorder.flgs.IsReadyToSave = 0;
		}
	}
}

