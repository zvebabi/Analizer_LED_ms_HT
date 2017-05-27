/**
 * Analizer.cpp
 * \author LED Microsensor
 */

#ifndef ANALIZER_H_
#define ANALIZER_H_

#include "Arduino.h"
#include <stdio.h>
#include <stdint.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <SPI.h>

#define GeneratorPin 9
#define ShiftRegisterDelay 1
#define DACDelay 1
#define WBDelay  1
#define DISCHARGE_DELAY 50
#define PULSE_DELAY 25
#define REFERENCE_V 3000.0
#define NUM_OF_LED 45
#define NUM_OF_ETALON 3
#define MAX_PULSE_WIDTH 60
#define MAX_CURRENT 1000

//define SPI pins
#define ADC_PORT PORTF
#define DAC_PORT PORTF
#define PREAMP_PORT PORTF
#define EEPROM_PORT PORTF
#define SS_ADC PF6
#define SS_PREAMP PF7
#define SS_DAC PF5
#define SS_EEPROM PF4

//define Shift reg pins
#define SR_ENABLE PD0 //PB7
#define SR_CLR PD3 //PD4
#define SR_DATA PD2 //PD7
#define SR_CLK PD1 //PD6

//define indicator pins
#define SR_IND_ENABLE PB6
#define SR_IND_CLR PD7
#define SR_IND_DATA PD6
#define SR_IND_CLK PD5
#define WHITE_LED PB7

//define sample and hold pins
#define SH_PORT PORTD
#define SH_SET PD0
#define SH_RESET PD1

//pulse pin
#define GEN1 OCR1A //ir led
#define GEN2 OCR1B //indicator
#define GEN3 OCR1C //single indicator

#define BTN PE6
/**
 *
 */
typedef struct
{      //first digit n of resistor, 2nd digit - number of LED group
	float g1_1;
	float g2_1;
	float g1_2;
	float g2_2;
	float k[NUM_OF_LED];
}  etalon_t ;

/**
 *
 */
typedef struct
{
    uint16_t curr1;
    uint16_t curr2;
} current_t;

/**
 * allocate eeprom variable
 */
uint8_t EEMEM _empty[20] = {0xF};
uint16_t EEMEM _pulseWidth = 120;
float EEMEM _c_R = 3.9;
current_t EEMEM _pairsOfCurrent[NUM_OF_LED] = {
		140, 0,  //1-2
		90, 240,
		70, 370,
		40, 400,

		450, 0,  //2-3
		400, 40,
		400, 65,
		400, 80,

		140, 0,  //3-4
		115, 45,
		90, 75,
		60, 100,

		130, 0,  //4-5
		100, 70,
		75, 110,
		45, 150,

		180, 0,  //5-6
		150, 80,
		110, 140,
		70, 200,

		250, 0, //6
		0, 0, //21 ski
		0, 0, //22
		0, 0, //23

		26, 0, //24 // 7-8
		19, 7,
		11, 13,
		6, 20,

		28, 0, // 8-9
		20, 8,
		13, 15,
		7, 23,

		32, 0,  // 9-10
		23, 10,
		15, 18,
		8, 27,

		35, 0,  // 10-11
		27, 8,
		18, 15,
		10, 24,

		33, 0,  // 11-12
		24, 14,
		15, 28,
		8, 40,

		53, 0 //12
};
															     //  g1_1 g2_1, g1_2 g2_2
etalon_t EEMEM _etalons[NUM_OF_ETALON] = {50,    10,       30,      10,
		1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,
				1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1};
float EEMEM _coefficients[NUM_OF_LED] = {1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,1,1,	1,1,1,1,1};

//define sram variable
//volatile uint16_t pulseWidth;
volatile current_t cur4AllLed[NUM_OF_LED];
volatile float coeffs[NUM_OF_LED];
volatile bool oneTimes = false;
volatile bool pulseEnd =false;
/**
 * This function set coeffs of OpAmp
 * Control digital potentiometers ad5141
 * Max value is 100kOhm, with step 0.39kOhm
 * After resistance will set, it send a message with currently setted values to serial port
 * @param RWB1 1st cascade (current to voltage)
 * @param RWB2 2nd cascade (signal amplification)
 */
void setPreAmp(float RWB1, float RWB2);

/**
 * This function set currents for LED in channel channelN
 * Control DAC
 * Max value is 1000mA, with step 1mA
 * After current will set, it send a message with currently setted values to serial port
 * @param channelN  - Number of channel 1 or 2
 * @param currValue - DAC output voltage, max 1000mV it is equal 1000mA on LED
 */
void setCurrent(uint8_t channelN, uint16_t currValue);

/**
 * Set width of impulse in microseconds
 * step is 1us, max 200us
 * @param width  - time of pulse, max 200us
 */
void setPulseWidth(uint16_t width);

/**
 * Save to eeprom value of input resistanse of second OpAmp
 * @param val
 */
void setC_R(float val);

/**
 * Make one pulse to the LED with previously setted current and time of pulse
 */
void doOnePulse(uint16_t pulseWidth);

/**
 * Discharge capasitor, while measurements going in Sample&Hold mode
 */
void dischargeSampleHold();

/**
 * Utility function for calibration mode
 */
void disableLED();

/**
 * Main calibration function
 *
 */
void factoryCalibr();

/**
 * Amplifier calibration function
 */
void preAmpCalibr();

/**
 * One mode of measurements with option of coeffs calibration
 * Manually choosing preamps parameters
 * Sample&Holde mode
 * @param numOfEtalon - which etalon we use for fast calibration
 * @param calcNorm - true - calibration, false - measurements
 */
void doMeasurementsSH(uint8_t numOfEtalon=0, bool calcNorm=false);

/**
 * One mode of measurements with option of coeffs calibration
 * Series of measurements with predefined preamp parameters
 * Sample&Holde mode
 * @param calcNorm - true - calibration, false - measurements
 */
void doMeasurementsSH_Avg(bool calcNorm=false);

/**
 * One mode of measurements with option of coeffs calibration
 * Manually choosing preamps parameters
 * Inpulse measurements
 * @param[in] numOfEtalon - which etalon we use for fast calibration
 * @param[in] calcNorm - true - calibration, false - measurements
 */
void doMeasurements(uint8_t numOfEtalon=0, bool calcNorm=false);

/**
 * Utility function for inpulse mode of measurements
 * @param[out] value buffer for reading adc value (w/o convert to mV)
 */
inline void readADCOneTime(uint16_t& value);

/**
 * Utility function for Sample&Holde mode of measurements
 * make 16 samples and calc average,
 * @param[out] value buffer for reading voltage (mV)
 */
void readADC(float& value);

/**
 * Print all saved to eeprom data to serial port
 */
void writeConfigToUart();

/**
 *  Utility function for work with shift register
 *  Set all outputs to High level
 */
void shiftRegisterReset();

/**
 *  Utility function for work with shift register
 *  Select next led to ON
 */
void shiftRegisterNext();

/**
 *  Utility function for work with shift register
 *  Select first pair of led
 */
void shiftRegisterFirst();

/**
 *  Utility function for work with shift register
 *  Set all outputs to High level
 */
void indicatorRegisterReset();

/**
 *  Utility function for work with shift register
 *  Select next led to ON
 */
void indicatorRegisterNext();

/**
 *  Utility function for work with shift register
 *  Select first pair of led
 */
void indicatorRegisterFirst();

void indicatorBlinc();
void singleLEDBlinc();

/**
 * Utility function for cleanserial port buffer
 */
void SerialClean();


/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Функция задания величины регистра OCR3A в [us]
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
PulseWidth            :   Длительность импульса [us]: 0.0625 - 4000, кратно 0.0625 us.
---------------------------------------------------------------------------------------------------------------------------*/
void Set_OCR3A(float PulseWidth) {
	PulseWidth = (PulseWidth > 125) ? 125 : PulseWidth;
	PulseWidth = (PulseWidth < 1) ? 1 : PulseWidth;
	OCR0A = (unsigned int) (PulseWidth*2);
}
/**
 * Main initialization of MCU
 */
void setup();

/**
 *  Infinite cycle
 *  Will perform after startup device
 */
void loop();

extern "C" void __cxa_pure_virtual()
{
	while(1);
}

#endif /* ANALIZER_H_ */

