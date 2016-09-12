/*
 * init.c
 *
 *  Created on: 26 de ago de 2016
 *      Author: andre
 */
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/systick.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/timer.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "fatfs/ff.h"
#include "utils/uartstdio.h"
#include "waverecorder.h"
#include "rtc.h"

FATFS Volume;

void initTimer1()
{
	unsigned long ulPeriod = 0;
	unsigned long clock = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);

	clock = SysCtlClockGet();
	ulPeriod = clock* TIME_AQUISITION;

	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER1_BASE, TIMER_A, ulPeriod -1);
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
	IntEnable(INT_TIMER1A);

}


int initVolume()
{
	FRESULT result;

	result = f_mount( &Volume, "", 1);
	if( result == FR_OK )
	{
		initTimer1();
	}

	return result;
}


void initGpios()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
}

void initTimer0()
{
	unsigned long ulPeriod = 0;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	ulPeriod = ( SysCtlClockGet() / SAMPLE_RATE);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
	TimerLoadSet(TIMER0_BASE, TIMER_A, ulPeriod -1);
	TimerControlTrigger(TIMER0_BASE,TIMER_A,true);
	TimerEnable(TIMER0_BASE, TIMER_A);
}

void initADC0_3()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	SysCtlADCSpeedSet(SYSCTL_ADCSPEED_1MSPS);
	ADCSequenceDisable(ADC0_BASE, 3);

	ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_TIMER, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END );
	ADCSequenceEnable(ADC0_BASE, 3);

	ADCIntEnable(ADC0_BASE,3);
	IntEnable(INT_ADC0SS3);

}


void initRTC()
{
	SysTickPeriodSet(10000000); //max 16mio
	SysTickIntRegister(RTCIntHandler);
	SysTickIntEnable();
	SysTickEnable();
}


void initUART1()
{

	SysCtlPeripheralEnable(SYSCTL_PERIPH2_UART1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH2_GPIOB);

	GPIOPinConfigure(GPIO_PB0_U1RX);
	GPIOPinConfigure(GPIO_PB1_U1TX);
	GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1 );

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE );
	UARTStdioInitExpClk(1,115200);
}

void init(WAVE_REC_DEF* WaveRec)
{
	//configure the system clock to run at 80MHz
	WaveRec->flgs.Active = 0;
	WaveRec->buff.active = WaveRec->buff._0;
	WaveRec->buff.toSave = WaveRec->buff._1;
	WaveRec->buff.size = sizeof(*WaveRec->buff._0)*WAVE_BUFFER_SIZE;
	WaveRec->flgs.IsReadyToSave = 0;

	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
	initGpios();
	initADC0_3();
	initTimer0();
	initRTC();
	initUART1();
	initVolume();
}

