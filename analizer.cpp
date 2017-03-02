/*
 * main.cpp
 *
 *  Created on: Jan 29, 2017
 *      Author: zvebabi
 */

//#include <avr/io.h>
#include "Arduino.h"
#include <stdio.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <SPI.h>
#include "pinsDef.h"


typedef struct
{
	float k[NUM_OF_LED];
	float g1;
	float g2mid;
	float g2min;
	float g2max;
}  etalon_t;

typedef struct
{
    uint16_t curr1;
    uint16_t curr2;
} current_t;

//allocate eeprom variable
uint8_t EEMEM _empty[20] = {0xF};
uint16_t EEMEM _pulseWidth = 80;
float EEMEM _c_R = 3.9;
current_t EEMEM _pairsOfCurrent[NUM_OF_LED];
etalon_t EEMEM _etalons[NUM_OF_ETALON];
float EEMEM _coefficients[NUM_OF_LED];

//define sram variable
volatile uint16_t pulseWidth;
//volatile float c_R;
volatile current_t cur4AllLed[NUM_OF_LED];
//volatile etalon_t etalons[NUM_OF_ETALON];
volatile float coeffs[NUM_OF_LED];
volatile bool oneTimes = false;
volatile bool pulseEnd =false;

void setPreAmp(float RWB1, float RWB2);
void SerialClean();
void setCurrent(uint8_t channelN, uint16_t currValue);
void setPulseWidth(uint16_t width);
void setC_R(float val);
void doOnePulse();
void dischargeSampleHold();
void disableLED();
void factoryCalibr();
void preAmpCalibr();
void doMeasurementsSH(uint8_t numOfEtalon=0, bool calcNorm=false);
void doMeasurementsSH_Avg(bool calcNorm=false);
void doMeasurements(uint8_t numOfEtalon=0, bool calcNorm=false);
inline void readADCOneTime(uint16_t& value);
void readADC(float& value);
void writeConfigToUart();
void shiftRegisterReset();
void shiftRegisterNext();
void shiftRegisterFirst();

ISR(TIMER1_COMPA_vect)
{
	if (oneTimes)
		GEN1 = 0;
	pulseEnd= true;
}

void setup();
void loop();

extern "C" void __cxa_pure_virtual()
{
	while(1);
}

int main(void)
{
	init();
	USBDevice.attach();
	setup();
	//endless loop
	for (;;) {
		loop();
	}
	return 0;
}

void setup() {
	//---setup Timer 1------
	DDRB |=  (1 << PB5) | (1 << PB6); //Gen_1 Gen_2
	PORTB &= ~((1 << PB5)|(1 << PB6));

	TCCR1A = (2 << COM1A0)| (3<< WGM10);  // Clear on compare match. prescaler 8,  fastpwm 10bit
	TCCR1B = (1 << WGM12) | (2 << CS10); // T=512us=1024indixies
	TCNT1 = 0; // Timer Clear
	GEN1 =  0;  // PreSet pulse width // 1index ==0.5us Max width is 100us=200 indicies
	TIMSK1 = (1 << OCIE1A) ; // Set interrupt on Compare A Match and Timer Overflow interrupt

	//init serial port
	Serial.begin(115200);
	_delay_ms(1500);

	//-------SS Pins ----------
	DDRF |=  (1 << SS_DAC);
	DDRB |=  (1 << SS_PREAMP);
	DDRE |=  (1 << SS_ADC);
	PORTF |= (1 << SS_DAC);
	PORTB |= (1 << SS_PREAMP);
	PORTE |= (1 << SS_ADC);

	//Sample-and-Holdl-Pins
	DDRD |=  (1 << SH_SET)|(1 << SH_RESET);
	PORTD &= ~((1 << SH_SET) | (1 << SH_SET));

	//shift registr init
	DDRB |= (1<< SR_ENABLE);
	DDRD |= (1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA);
	PORTB |= (1 << SR_ENABLE);      //OE HIGH - disable
	PORTD &= ~((1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA));   //set  SRCLR / RCLK / SER -LOW

	//initialize SPI
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE2);
	SPI.begin();

	Serial.print(F("Reading config... "));
	pulseWidth =	eeprom_read_word(&_pulseWidth);
	eeprom_read_block((void *)&cur4AllLed, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
	eeprom_read_block((void *)&coeffs, (const void *) &_coefficients, NUM_OF_LED*sizeof(float));
	Serial.print(F("Done!\n\n"));
	writeConfigToUart();

	shiftRegisterReset();
//	shiftRegisterFirst();
	PORTB &= (~(1 << SR_ENABLE));  //OE HIGH - enble shiftreg
	Serial.print(F("First led is on!\n"));
}

void loop()
{
	Serial.flush();
	_delay_ms(5);			//debug

	//Определение типа команды
	if (Serial.available() > 0)
	{
		char OperationCode;
		OperationCode = Serial.read();
		switch (OperationCode)
		{
			case 'q': //main calibration function
			{
				factoryCalibr();
				break;
			}
			case 'e': //express calibration
			{
				Serial.print(F("Not implemented yet e\n"));
				break;
			}
			case 's': // start measurements sample and hold
			{
				doMeasurementsSH();
				break;
			}
			case 'm': // start measurements in pulse mode
			{
				doMeasurements();
				break;
			}
			case 'a': // start measurements in sample and hold mode for all
			{
				doMeasurementsSH_Avg();
				break;
			}

			default:
			{
				SerialClean();
				break;
			}
		}
		SerialClean();
	}
}

/*
 * go to factory calibr mode
 */
void factoryCalibr()
{
	bool calibrEnd = false;
	uint8_t numLed=0;
	SerialClean();
	Serial.print(F("You are in calibration mode!\n"));
	Serial.print(F("Firstly recalibrate LEDs' current.\n"));
	shiftRegisterReset();

	//infinite loop for calibrating
	while (!calibrEnd)
	{
		Serial.flush();
		//Определение типа команды
		if (Serial.available() > 0)
		{
			char OperationCode;
			OperationCode = Serial.read();
			switch (OperationCode)
			{
				case 'w': //set pulsewidth
				{
					uint16_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					setPulseWidth(curValue);
					Serial.print(F("Set pulse width to: "));
					Serial.println(curValue);
					break;
				}
				case 't': //set pulsewidth
				{
					float curValue = 0;
					curValue = Serial.parseFloat(SKIP_WHITESPACE);
					setC_R(curValue);
					Serial.print(F("Set input resistanse to: "));
					Serial.println(curValue);
					break;
				}
				case 'a': 	//-----Set current channel ONE-----
				{
					uint8_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					cur4AllLed[numLed].curr1 = curValue;
					setCurrent(1, curValue);
					break;
				}
				case 'b':	//-----Set current channel TWO-----
				{
					uint8_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					cur4AllLed[numLed].curr2 = curValue;
					setCurrent(2, curValue);
					break;
				}
				case 'g':  //reset LEDs
				{
					disableLED();
					numLed=0;
					shiftRegisterReset();
					Serial.print(F("All LEDs is off.\nNow you can restart calibration.\n"));
					break;
				}
				case 'f':  //light-up first led
				{
					disableLED();
					numLed=0;
					shiftRegisterFirst();
					Serial.print(F("First led is on!\n"));
					break;
				}
				case 'n':  //next led choose
				{
					disableLED();
					if (numLed == NUM_OF_LED-1)
					{ //TODO: do correct exit, return to first led or just end.
						Serial.print("LED ");
						Serial.print(numLed+1); //actually, numeration starts from zero
						Serial.print(" still is on!\n");  //here we will see the classic numeration from 1
						Serial.print(F("This is end of calibration\n"));
					}
					else
					{
						++numLed;
						if(numLed == 21 )
							numLed = 24;
						if (numLed%4 == 0)     //light up next group of led
							shiftRegisterNext();  //
						Serial.print("LED ");
						Serial.print(numLed+1); //actually, numeration starts from zero
						Serial.print(" is on!\n");  //here we will see the classic numeration from 1
					}
					break;
				}
				case 'o':  //take infinite series of pulse to LED
				{
					oneTimes = false;
					GEN1=pulseWidth;
					TCNT1 = 0;
					break;
				}
				case 'p':  //take one pulse to LED
				{
					oneTimes = true;
					GEN1=pulseWidth;
					TCNT1 = 0;
					break;
				}
				case 'i': //disable LED
				{
					disableLED();
					break;
				}
				case 'z':	//return current led number
				{
					Serial.print(F("Current LED is "));
					Serial.println(numLed+1);
					break;
				}
				case 'c':	//-----Set resistance -----
				{
					float resVal1;
					float resVal2;
					resVal1 = Serial.parseFloat(SKIP_WHITESPACE);
					resVal2 = Serial.parseFloat(SKIP_WHITESPACE);
					setPreAmp(resVal1,resVal2);
					break;
				}
				case 'r':	//Go to preamp calibration
				{
					preAmpCalibr();
					break;
				}
				case 's':	// Save parameters for all LED to !!!EEPROM!!!
				{
					eeprom_write_block((const void *)&cur4AllLed, (void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
					writeConfigToUart();
					break;
				}
				case 'e':	// End Calibr
				{
					writeConfigToUart();
					Serial.print(F("You leave the calibration mode!\nGoodBye!"));
					calibrEnd = true;
					break;
				}
				case 'd': //random init
				{
					etalon_t etalons[NUM_OF_ETALON];
					setPulseWidth(100);
					setC_R(3.9);
					for (uint8_t i=0;i< NUM_OF_LED; i++)
					{
						if(i==21) {i=24;}
						cur4AllLed[i].curr1 =20+i;
						cur4AllLed[i].curr2 =30+i;
						coeffs[i] = 1.1;
					}
					for (uint8_t k=0;k<NUM_OF_ETALON; k++)
					{
						etalons[k].g1 = 50+k;
						etalons[k].g2mid = 50+k;
						etalons[k].g2max = 80+k;
						etalons[k].g2min = 20+k;
						for (uint8_t i=0;i< NUM_OF_LED; i++)
						{
							if(i==21) {i=24;}
							etalons[k].k[i] = 1.2;
						}
					}
					eeprom_write_block((const void *)&cur4AllLed, (void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
					eeprom_write_block((const void *)&coeffs, (void *) &_coefficients, NUM_OF_LED*sizeof(float));
					eeprom_write_block((const void *)&etalons, (void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
					writeConfigToUart();
					break;
				}
				default:
				{
					SerialClean();
					break;
				}
			}
		}
	}
}

//write etalondata to memory
void preAmpCalibr()
{
	bool calibrEnd = false;
	uint8_t numEtalon=0;
	etalon_t etalons[NUM_OF_ETALON];
	Serial.print(F("You are in PreAmplifier calibration mode!\n"));
	shiftRegisterReset();
	//infinite loop for calibrating
	while (!calibrEnd)
	{
		Serial.flush();
		//Определение типа команды
		if (Serial.available() > 0)
		{
			char OperationCode;
			OperationCode = Serial.read();
			switch (OperationCode)
			{
				case 'c':	//-----Set resistance -----
				{
					float resVal1;
					float resVal2;
					resVal1 = Serial.parseFloat(SKIP_WHITESPACE);
					resVal2 = Serial.parseFloat(SKIP_WHITESPACE);
					setPreAmp(resVal1,resVal2);
					resVal1 = (resVal1 > 100.0) ? 100.0 : ((resVal1 < 0) ? 0 : resVal1);
					resVal2 = (resVal2 > 100.0) ? 100.0 : ((resVal2 < 0.0) ? 0.0 : resVal2);
					etalons[numEtalon].g1 = resVal1;
					etalons[numEtalon].g2mid = resVal2;
					break;
				}
				case 'x': //choose etalon
				{
					uint8_t index;
					do {
						Serial.print(F("Set etalon # \n"));
						index = Serial.parseInt(SKIP_WHITESPACE);
					}
					while (index > NUM_OF_ETALON && index < 1);
					numEtalon = index-1;
					Serial.print(F("Selected Etalon cell is "));
					Serial.println(index);
					break;
				}
				case 'e':	// End Calibr
				{
					writeConfigToUart();
					Serial.print(F("You leave the PreAmplifier calibration mode!\nGoodBye!"));
					calibrEnd = true;
					break;
				}
				case 's':	// Save parameters  to !!!EEPROM!!!
				{
					eeprom_write_block((const void *)&etalons, (void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
					writeConfigToUart();
					break;
				}
				case 'z':	//return current etalon number
				{
					Serial.print(F("Current etalon is "));
					Serial.println(numEtalon+1);
					break;
				}
				case 'k': //save k for etallon
				{
					float curVal;
					for (uint8_t j=0; j< NUM_OF_LED; j++)
					{
						if(j==21) 	j=24; //skip 6-7
						curVal = Serial.parseFloat(SKIP_WHITESPACE);
						etalons[numEtalon].k[j] = curVal;
						Serial.print(F("LED# "));
						Serial.print(j+1);
						Serial.print(F(", k is "));
						Serial.println(curVal, 3);
					}
					break;
				}
				case 'm': //set min and max value of g2
				{
					float curVal;
					curVal = Serial.parseFloat(SKIP_WHITESPACE);
					etalons[numEtalon].g2min = curVal;
					Serial.print(F("Min is "));
					Serial.println(curVal);
					curVal = Serial.parseFloat(SKIP_WHITESPACE);
					etalons[numEtalon].g2max = curVal;
					Serial.print(F("Max is "));
					Serial.println(curVal);
					break;
				}
				case 'n':  //calc Ai
				{
					float coeff_temp[NUM_OF_LED];
					eeprom_read_block((void *)coeff_temp, (const void *)_coefficients, NUM_OF_LED*sizeof(float));
					doMeasurementsSH(numEtalon, true);
					Serial.print(F("Ai#, Previous , Current, Diff(%)\n"));
					for (uint8_t i=0; i< NUM_OF_LED; i++)
					{
						if(i==21) i=24;
						Serial.print(i);
						Serial.print(F(", "));
						Serial.print(coeff_temp[i]); 	//previous
						Serial.print(F(", "));
						Serial.print(coeffs[i]); 			//current
						Serial.print(F(", "));
						Serial.print( (coeffs[i] - coeff_temp[i] ) / coeffs[i] * 100.0); //diff
						Serial.print(F("\n"));
					}
					Serial.print(F("If params is ok - press \"y\".\nFor reset press any key.\n"));
					while(Serial.available()==0){_delay_ms(1);} //wait for command
					if (Serial.read() == 'y')
						eeprom_write_block((const void *)coeffs, (void *)_coefficients, NUM_OF_LED*sizeof(float)); //save params from last measurement
					else
						eeprom_read_block((void *)coeffs, (const void *)_coefficients, NUM_OF_LED*sizeof(float)); //reset params in coeffs
					break;
				}
				case 'a': //calc Ai avg method
				{
					float coeff_temp[NUM_OF_LED];
					eeprom_read_block((void *)coeff_temp, (const void *)_coefficients, NUM_OF_LED*sizeof(float));
					doMeasurementsSH_Avg(true);
					Serial.print(F("Ai#, Previous , Current, Diff(%)\n"));
					for (uint8_t i=0; i< NUM_OF_LED; i++)
					{
						if(i==21) i=24;
						Serial.print(i);
						Serial.print(F(", "));
						Serial.print(coeff_temp[i]); 	//previous
						Serial.print(F(", "));
						Serial.print(coeffs[i]); 			//current
						Serial.print(F(", "));
						Serial.print( (coeffs[i] - coeff_temp[i] ) / coeffs[i] * 100.0); //diff
						Serial.print(F("\n"));
					}
					Serial.print(F("If params is ok - press \"y\".\nFor reset press any key.\n"));
					while(Serial.available()==0){_delay_ms(1);} //wait for command
					if (Serial.read() == 'y')
						eeprom_write_block((const void *)coeffs, (void *)_coefficients, NUM_OF_LED*sizeof(float)); //save params from last measurement
					else
						eeprom_read_block((void *)coeffs, (const void *)_coefficients, NUM_OF_LED*sizeof(float)); //reset params in coeffs
					break;
				}
				case 'l': //save Ai in manual mode
				{
					float coeff_temp[NUM_OF_LED];
					float curVal;
					for (uint8_t j=0; j< NUM_OF_LED; j++)
					{
						if(j==21) 	j=24; //skip 6-7
						curVal = Serial.parseFloat(SKIP_WHITESPACE);
						coeffs[j] = curVal;
						Serial.print(F("Ai# "));
						Serial.print(j+1);
						Serial.print(F(", A is "));
						Serial.println(curVal, 3);
					}
					eeprom_write_block((const void *)coeff_temp, (void *)_coefficients, NUM_OF_LED*sizeof(float));
					break;
				}
				default:
				{
					SerialClean();
					break;
				}
			}
		}
	}
}

void doMeasurements(uint8_t numOfEtalon, bool calcNorm)
{
	float measuredU[NUM_OF_LED];
	float value = 0;
	float background=0;
	float k=0; // norm coeefs or result
	uint8_t index;
	Serial.print(F("You are in measurements mode!\n"));
	if (!calcNorm)
	{
		do {
			Serial.print(F("Set etalon # \n"));
			while (Serial.available() == 0) {_delay_ms(1);}
			index = Serial.parseInt(SKIP_WHITESPACE);
		}
		while (index > NUM_OF_ETALON && index < 1);
		--index; //convert to programmers numering
	}
	else
		index = numOfEtalon;
	etalon_t etalonForCalc;
	float c_R = eeprom_read_float(&_c_R); //input resistanse of g2
	eeprom_read_block((void *)&etalonForCalc, (const void *)&_etalons[index], sizeof(etalon_t));
	float constant = calcNorm ? (etalonForCalc.g1 * etalonForCalc.g2mid / c_R) :(etalonForCalc.g1 * etalonForCalc.g2mid/c_R) ;

	setPreAmp(etalonForCalc.g1, etalonForCalc.g2mid);
	//measurements
	shiftRegisterReset();
	disableLED();
	shiftRegisterFirst(); //select first led pair
	for (uint8_t i = 0; i< NUM_OF_LED; i++)
	{
		if( i == 21 ) i = 24;//skip 6-7 leds combinations
		if( i != 0 && i % 4 == 0) // !=0 for first led exception
			shiftRegisterNext();
		dischargeSampleHold(); //reset Sample and hold
		//read background value
		readADC(background);
		setCurrent(1, cur4AllLed[i].curr1);
		setCurrent(2, cur4AllLed[i].curr2);
		doOnePulse();
		uint32_t adcAvgData=0 ;
		uint16_t adcOneTimeData;
		uint8_t counter=0;
		while(!pulseEnd)
		{
			readADCOneTime(adcOneTimeData);
			adcAvgData += adcOneTimeData;
			counter++;
		}
		adcAvgData /=counter;
		value = adcAvgData * (REFERENCE_V/32767.0); //convert to mv
		measuredU[i] = value-background;
		disableLED();
	}
	//calc k
	if (calcNorm) // part of express calibration
	{
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			k = measuredU[i] / ( etalonForCalc.k[i] * constant);
			coeffs[i] = k;
		}
		Serial.print(F("Coefficients are ready to save.\n"));
	}
	else //calc k of sample
	{
		Serial.print(F("LED#,k,Uamp(mV)\n"));
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			Serial.print(i);
			Serial.print(F(","));
			k = measuredU[i] / ( coeffs[i] * constant);
			Serial.print(k);
			Serial.print(F(","));
			Serial.println(measuredU[i]);
		}
	}
}
/*
 * Sample and hold version
 */
void doMeasurementsSH(uint8_t numOfEtalon, bool calcNorm)
{
	float measuredU[NUM_OF_LED];
	float value = 0;
	float background=0;
	float k=0; // norm coeefs or result
	uint8_t index;
	Serial.print(F("You are in measurements mode!\n"));
	if (!calcNorm)
	{
		do {
			Serial.print(F("Set etalon # \n"));
			while (Serial.available() == 0) {_delay_ms(1);}
			index = Serial.parseInt(SKIP_WHITESPACE);
		}
		while (index > NUM_OF_ETALON && index < 1);
		--index; //convert to programmers numering
	}
	else
		index = numOfEtalon;

	etalon_t etalonForCalc;
	eeprom_read_block((void *)&etalonForCalc, (const void *)&_etalons[index], sizeof(etalon_t));
	float c_R = eeprom_read_float(&_c_R); //input resistanse of g2
	//
	float constant = calcNorm ? (etalonForCalc.g1 * etalonForCalc.g2mid / c_R) :(etalonForCalc.g1 * etalonForCalc.g2mid/c_R) ;

	setPreAmp(etalonForCalc.g1, etalonForCalc.g2mid);
	//measurements
	shiftRegisterReset();
	disableLED();
	shiftRegisterFirst(); //select first led pair
	for (uint8_t i = 0; i< NUM_OF_LED; i++)
	{
		if( i == 21 ) i = 24;//skip 6-7 leds combinations
		if( i != 0 && i % 4 == 0) // !=0 for first led exception
			shiftRegisterNext();
		dischargeSampleHold(); //reset Sample and hold
		//read background value
		readADC(background);
		setCurrent(1, cur4AllLed[i].curr1);
		setCurrent(2, cur4AllLed[i].curr2);
		doOnePulse();
		while(!pulseEnd) {_delay_us(1);}
		readADC(value); //convert to mV
		measuredU[i] = value-background;
		disableLED();
	}
	//calc k
	if (calcNorm) // part of express calibration
	{
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			k = measuredU[i] / ( etalonForCalc.k[i] * constant);
			coeffs[i] = k;
		}
		Serial.print(F("Coefficients are ready to save.\n"));
	}
	else //calc k of sample
	{
		Serial.print(F("LED#,k,Uamp(mV)\n"));
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			Serial.print(i);
			Serial.print(F(","));
			k = measuredU[i] / ( coeffs[i] * constant);
			Serial.print(k);
			Serial.print(F(","));
			Serial.println(measuredU[i]);
		}
	}
}

/*
 * Sample and hold version
 */
void doMeasurementsSH_Avg(bool calcNorm)
{
	Serial.print(F("You are in measurements mode!\n"));

	float kAvg[NUM_OF_LED] = {0.0};
	float c_R = eeprom_read_float(&_c_R); //input resistanse of g2

	for(uint8_t index = 0; index < NUM_OF_ETALON; index++)
	{
		float k=0; // norm coeefs or result
		float measuredU[NUM_OF_LED] ;
		float value = 0;
		float background=0;
		//read config
		etalon_t etalonForCalc;
		eeprom_read_block((void *)&etalonForCalc, (const void *)&_etalons[index], sizeof(etalon_t));
		float constant = etalonForCalc.g1 * etalonForCalc.g2mid / c_R ;
		setPreAmp(etalonForCalc.g1, etalonForCalc.g2mid);
		//reset led and light up first one
		shiftRegisterReset();
		disableLED();
		shiftRegisterFirst();
		//one measurement
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if( i == 21 ) i = 24;//skip 6-7 leds combinations
			if( i != 0 && i % 4 == 0) // !=0 for first led exception
				shiftRegisterNext();
			dischargeSampleHold(); //reset Sample and hold
			//read background value
			readADC(background);
			setCurrent(1, cur4AllLed[i].curr1);
			setCurrent(2, cur4AllLed[i].curr2);
			doOnePulse();
			while(!pulseEnd) {_delay_us(1);}
			readADC(value); //convert to mV
			measuredU[i] = value-background;
			disableLED();
		}
		if (calcNorm) // part of express calibration
			Serial.print(F("LED#,Ai,Uamp(mV)\n"));
		else
			Serial.print(F("LED#,k,Uamp(mV)\n"));
//calc k
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			if(calcNorm)
			{
				k = measuredU[i] / ( etalonForCalc.k[i] * constant);
			}
			else
			{
				k = measuredU[i] / ( coeffs[i] * constant);
			}
//print result
			Serial.print(i);
			Serial.print(F(","));
			Serial.print(k);
			Serial.print(F(","));
			Serial.println(measuredU[i]);
//calc avg
			kAvg[i]+=k;
		}
	}

	Serial.print(F("\nAverage values.\n"));
	if (calcNorm) // part of express calibration
		Serial.print(F("LED#,Ai,\n"));
	else
		Serial.print(F("LED#,k\n"));
//calc and print average value
	for(uint8_t i = 0; i< NUM_OF_LED; i++)
	{
		kAvg[i] /=NUM_OF_ETALON;
		Serial.print(i);
		Serial.print(F(","));
		Serial.println(kAvg[i]);
		if(calcNorm)  // part of express calibration
			coeffs[i] = kAvg[i];
	}
}

void readADCOneTime(uint16_t& value)
{
	ADC_PORT |= (1 << SS_ADC); //start conversion
	_delay_us(3);
	ADC_PORT &= ~(1<< SS_ADC); // set to low for start acquisition
	value = SPI.transfer16(0x0);
//	value = val*(REFERENCE_V/32767); //convert to mV
}

void readADC(float& value)
{
	uint32_t val=0;
	for (uint8_t i=0;i<8;i++)
	{
		uint16_t temp=0;
		ADC_PORT |= (1 << SS_ADC); //start conversion
		_delay_us(3);
		ADC_PORT &= ~(1<< SS_ADC); // set to low for start acquisition
		temp = SPI.transfer16(0x0);
		val+=temp;
	}
	val = val>>4; // devide by 16
	value = val * REFERENCE_V / 32767.0; //convert to mV
}

//write to usb(uart) all data saved to eeprom in CSV format
//save it in *.csv file
//Open in excel and you will see a ordinary table
void writeConfigToUart()
{
	Serial.print(F("This data loaded from EEPROM.\n"));
	//pulse
	uint16_t pulse_temp = eeprom_read_word(&_pulseWidth);
	Serial.print(F("\nPulse Width is: "));
	Serial.println(pulse_temp/2); //because PW in memory in ticks, not in us
	//resistance
	float res_temp = eeprom_read_float(&_c_R);
	Serial.print(F("\nInput resistance is: "));
	Serial.println(res_temp, 3);
	//current
	current_t cur4AllLed_temp[NUM_OF_LED];
	eeprom_read_block((void *)&cur4AllLed_temp, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
	Serial.print(F("\nCurrents:\n"));
	Serial.print(F("LED#, I1(mA), I2(mA)\n"));
	for (uint8_t i=0;i< NUM_OF_LED; i++)
	{
		if(i==21) {i=24;}
		Serial.print(i+1);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr1);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr2);
		Serial.print(F("\n"));
	}
	//preamp and etalon
	etalon_t etalons_temp[NUM_OF_ETALON];
	eeprom_read_block((void *)&etalons_temp, (const void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
	Serial.print(F("\nPreamp Data:\n"));
	Serial.print(F("Etalon#, g1, g2mid, g2min, g2max, k\n"));
	for (uint8_t i=0;i< NUM_OF_ETALON; i++)
	{
		Serial.print(i+1);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g1);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g2mid);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g2min);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g2max);
		for(uint8_t k=0; k<NUM_OF_LED;k++)
		{
			if(k==21) {k=24;}
			Serial.print(F(","));
			Serial.print(etalons_temp[i].k[k]);
		}
		Serial.print(F("\n"));
	}
	//coeffs
	float coeffs_temp[NUM_OF_LED];
	eeprom_read_block((void *)&coeffs_temp, (const void *) &_coefficients, NUM_OF_LED*sizeof(float));
	Serial.print(F("\nCoeefs Ai:\n"));
	Serial.print(F("LED#, Ai\n"));

	for (uint8_t i=0;i< NUM_OF_LED; i++)
	{
		if(i==21) i=24;
		Serial.print(i+1);
		Serial.print(F(","));
		Serial.print(coeffs_temp[i]);
		Serial.print(F("\n"));
	}
	Serial.print(F("This data loaded from EEPROM.\n"));
}

/*
 * Функция управления потенциометрами AD5141
 * ----------------------------------------------------------------------------------------------------------------------------
 * RWB1    :   установить сопротивление U3: от 0 до 100 кОм (кратно 0.39 кОм).
 * RWB2    :   установить сопротивление U4: от 0 до 100 кОм (кратно 0.39 кОм).
*/
void setPreAmp(float RWB1, float RWB2) {
	//check max-min border
	RWB1 = (RWB1 > 100.0) ? 100.0 : RWB1;
	RWB1 = (RWB1 < 0) ? 0 : RWB1;
	RWB2 = (RWB2 > 100.0) ? 100.0 : RWB2;
	RWB2 = (RWB2 < 0.0) ? 0.0 : RWB2;

	uint8_t RWB1_code = 0;
	uint8_t RWB2_code = 0;
	RWB1_code = (RWB1 * 255.0)/100;
	RWB2_code = (RWB2 * 255.0)/100;

	PREAMP_PORT &= (~(1 << SS_PREAMP));  //SS_AD5141 LOW
	_delay_us(WBDelay);
	cli();
	SPI.transfer(B00010000);    //Write to RDAC
	SPI.transfer(RWB2_code);
	SPI.transfer(B00010000);    //Write to RDAC
	SPI.transfer(RWB1_code);
	//read data from resistance
	SPI.transfer(B00110000);    //Read from RDAC
	RWB2_code = SPI.transfer(B00000011); //second
	SPI.transfer(B00110000);    //Read from RDAC
	RWB1_code = SPI.transfer(B00000011); //first
	_delay_us(WBDelay);
	PREAMP_PORT |= (1 << SS_PREAMP);     //SS_AD5141 HIGH
	_delay_us(WBDelay);
	Serial.print(F("Set resistanse to: \n"));
	Serial.println((RWB1_code*100/255.0),4);
	Serial.println((RWB2_code*100/255.0),4);

	sei();
}

/*
 * Функция управления DAC
 * ----------------------------------------------------------------------------------------------------------------------------
 * Channel       :   номер канала: 1(A) или 2(B).
 * Напряжение    :   устанавливаемое напряжение [mV]: от 0 до 2500 mVs.
 */
void setCurrent(uint8_t channelN, uint16_t curValue) {

	//-----initialize SPI------
	//SPI.setBitOrder(MSBFIRST);
	//SPI.setDataMode(SPI_MODE2);
	//SPI.begin();
	//check max-min border
	curValue = (curValue > MAX_CURRENT) ? MAX_CURRENT : curValue;
	curValue = (curValue < 0) ? 0 : curValue;
	//convert mV to index
	uint16_t index = (curValue * 65536) / 2500;

	DAC_PORT &= (~(1 << SS_DAC));   //SS_AD5689 LOW
	if (channelN == 1) {
		SPI.transfer(0b00110001);    //Write DAC_A
		SPI.transfer16(index);
	}
	if (channelN == 2) {
		SPI.transfer(B00111000);    //Write DAC_B
		SPI.transfer16(index);
	}
	DAC_PORT |= (1 << SS_DAC);      //SS_AD5689 HIGH
	DAC_PORT &= (~(1 << SS_DAC));   //SS_AD5689 LOW

	//readback data
	uint8_t address = (channelN == 2) ? 0b10011000 : 0b10010001;
	SPI.transfer(address);			//enable readback
	SPI.transfer16(0x0);

	DAC_PORT |= (1 << SS_DAC);      //SS_AD5689 HIGH
	DAC_PORT &= (~(1 << SS_DAC));   //SS_AD5689 LOW

	uint16_t savedValue=0;
	SPI.transfer(0x00);
	savedValue = SPI.transfer16(0x00);	//read
	Serial.print(F("Set current of channel "));
	Serial.print(channelN);
	Serial.print(F(" to: "));
	Serial.println(savedValue*2500.0/65536, 3);

	DAC_PORT |= (1 << SS_DAC);      //SS_AD5689 HIGH
}

//set width of impulse in microseconds
//step is 1us, max 200us
void setPulseWidth(uint16_t width)
{
	pulseWidth = width > MAX_PULSE_WIDTH ? MAX_PULSE_WIDTH : ( width < 0 ? 0 : width*2);
	eeprom_write_word(&_pulseWidth, pulseWidth); //save to eeprom
}

void setC_R(float val)
{
	eeprom_write_float(&_c_R, val); //save to eeprom
}

void doOnePulse()
{
	oneTimes = true;
	pulseEnd = false;
	GEN1=pulseWidth;
	TCNT1 = 0;
}

void dischargeSampleHold()
{
	SH_PORT &= ~(1<<SH_SET);
	SH_PORT |= (1<< SH_RESET);
	_delay_us(DISCHARGE_DELAY);
	SH_PORT &= ~(1<< SH_RESET);
	SH_PORT |= (1<<SH_SET);
}

//Включение управляющих сдвиговых регистров
//set register to 0xFFFF
void shiftRegisterReset() {
	//reset register
	PORTD &= (~(1 << SR_CLR));   //SRCLR-CLEAR-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_CLK);      //RCLK-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_CLK));   //RCLK-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_CLR);      //SRCLR-CLEAR-HIGH;

	PORTD |= (1 << SR_DATA);      //SER-HIGH
	for (uint8_t i = 0; i < 14; i++)
	{
		PORTD |= (1 << SR_CLK);      //RCLK-HIGH
		_delay_us(ShiftRegisterDelay);
		PORTD &= (~(1 << SR_CLK));   //RCLK-LOW
		_delay_us(ShiftRegisterDelay);
	}
	PORTD &= (~(1 << SR_DATA));   //SER-LOW
	_delay_us(ShiftRegisterDelay);
}

/*
 * Выбор следующей пары светодиодов
 */
void shiftRegisterNext() {
	PORTD |= (1 << SR_DATA);      //SER-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_CLK);      //RCLK-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_CLK));   //RCLK-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_DATA));   //SER-LOW
}

//Выбор первой пары светодиодов
void shiftRegisterFirst() {
	PORTD &= (~(1 << SR_DATA));   //SER-LOW
	_delay_us(ShiftRegisterDelay);
	for (uint8_t i = 0; i < 2; i++)
	{
		PORTD |= (1 << SR_CLK);      //RCLK-HIGH
		_delay_us(ShiftRegisterDelay);
		PORTD &= (~(1 << SR_CLK));   //RCLK-LOW
		_delay_us(ShiftRegisterDelay);
	}
	shiftRegisterNext();
}

void disableLED()
{
	setCurrent(1, 0);		//disable led
	setCurrent(2, 0);		//
	GEN1=0;
}

//Функция очистки serial monitor
void SerialClean() {
	while (Serial.available() > 0)
	{
		Serial.read();
	}
}
