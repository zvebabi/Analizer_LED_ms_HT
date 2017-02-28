/*
 * pinsDef.h
 *
 *  Created on: Jan 31, 2017
 *      Author: zvebabi
 */

#ifndef PINSDEF_H_
#define PINSDEF_H_
//#include <avr/io.h>

#define GeneratorPin 9
#define ShiftRegisterDelay 1
#define DACDelay 1
#define WBDelay  1000
#define DISCHARGE_DELAY 50
#define REFERENCE_V 3000.0
#define NUM_OF_LED 45
#define NUM_OF_ETALON 3
#define MAX_PULSE_WIDTH 100
#define MAX_CURRENT 1000

//define SPI pins
#define ADC_PORT PORTE
#define DAC_PORT PORTF
#define PREAMP_PORT PORTB
#define SS_ADC PE6
#define SS_PREAMP PB4
#define SS_DAC PF1

//define Shift reg pins
#define SR_ENABLE PB7
#define SR_CLR PD4
#define SR_DATA PD7
#define SR_CLK PD6

//define sample and hold pins
#define SH_PORT PORTD
#define SH_SET PD0
#define SH_RESET PD1

//pulse pin
#define GEN1 OCR1A
#define GEN2 OCR1B

#endif /* PINSDEF_H_ */

