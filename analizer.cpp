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
	uint8_t a;
	uint8_t b[10];
}  etalon;

typedef struct
{
    uint16_t curr1;
    uint16_t curr2;
} current_t;

//allocate eeprom variable
uint8_t EEMEM _empty[20];
uint16_t EEMEM _pulseWidth;
etalon EEMEM _etalonNum1;
etalon EEMEM _etalonNum2;
current_t EEMEM _pairsOfCurrent[NUM_OF_LED];

//define sram variable
volatile uint16_t pulseWidth;
volatile current_t cur4AllLed[NUM_OF_LED];
bool oneTimes = false;

void setPreAmp(float RWB1, float RWB2);
void SerialClean();
void setCurrent(uint8_t channelN, uint16_t currValue);
void setPulseWidth(uint16_t width);
void disableLED();
void factoryCalibr();
void preAmpCalibr();
void readConfigToUart();
void shiftRegisterReset();
void shiftRegisterNext();
void shiftRegisterFirst();

//-------------------------------------------------------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect)
{
	if (oneTimes)
	{
		GEN1 = 0;
	}
}
//-------------------------------------------------------------------------------------------------------------------------

#if 0
//-------------------------------------------------------------------------------------------------------------------------
ISR(TIMER3_COMPA_vect)
{
	switch(StageValMeasure)
	{
		case 0: // STAGE CONVERTION
		PORTE |= (1 << PORTE6);        // SS_AD7687 HIGH - Conversion start
		PORTD |= (1 << PORTD0);        // Sample start
		Set_OCR3A(20);
		StageValMeasure = 1;
		break;

		case 1: // STAGE ACQUISITION
		PORTE = 0;    // SS_AD7687 LOW - Acquisition start
		PORTD &= (~(1 << PORTD0));    //Sample stop
		TCCR3B = (1 << WGM32) | (0 << CS32) | (0 << CS31) | (0 << CS30); // set CTC-mode, scale to clock = *Timer STOPPED
		TCNT3 = 0;  // Timer clear
		Set_OCR3A(30);
		Data_ADC = SPI.transfer16(0x0);
		//Serial.println((Data_ADC*3000/32767),4);
		StageValMeasure = 0;
		break;
	}
}
//-------------------------------------------------------------------------------------------------------------------------
#endif

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
	GEN1 =  10;  // PreSet pulse width // 1index ==0.5us Max width is 100us=200 indicies
	TIMSK1 = (1 << OCIE1A) | (1 << TOIE1); // Set interrupt on Compare A Match and Timer Overflow interrupt

#if 0
	SetModeGenerator(60, 1000);   // 20 - длительность импульсов, 250 - период следования импульсов
	TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);   // set CTC-mode, prescaling =*1, timer START

	// ---Setup Timer2-----------------------------------------------------------------------------------------------------
	TCCR3A = (0 << COM3A1) | (0 << COM3A0);  // Set mode of pins: DISCONNECTED, set CTC-mode.
	TCCR3B = (1 << WGM32) | (0 << CS32) | (0 << CS31) | (0 << CS30); // set CTC-mode, scale to clock = *Timer STOPPED
	TCNT3 = 0; // Timer Clear
	TIMSK3 = (1 << OCIE3A); // Set interrupt on Compare A Match
	Set_OCR3A(50);
	TCCR3B = (1 << WGM32) | (0 << CS32) | (0 << CS31) | (1 << CS30);   // set CTC-mode, prescaling =*1, timer START
#endif
	//-------SS Pins ----------
	DDRF |=  (1 << SS_DAC);
	DDRB |=  (1 << SS_PREAMP);
	DDRE |=  (1 << SS_ADC);
	PORTF |= (1 << SS_DAC);
	PORTB |= (1 << SS_PREAMP);
	PORTE |= (1 << SS_ADC);

	//Sample-and-Holdl-Pins
	DDRD |=  (1 << SH_SET)|(1 << SH_RESET);
	PORTD &= ~((1 << PD0) | (1 << PD1));

	//shift registr init
	PORTB |= (1 << SR_ENABLE);      //OE HIGH - disable
	PORTD &= ~((1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA));   //set  SRCLR / RCLK / SER -LOW
	DDRB |= (1<< SR_ENABLE);
	DDRD |= (1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA);

	//initialize SPI
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE2);
	SPI.begin();

	//LED-ON
	setCurrent(1, 0);
	setCurrent(2, 0);
	shiftRegisterReset();
//	ShiftRegisterFirst();
//	PORTB &= (~(1 << SR_ENABLE));   //OE LOW
	_delay_ms(100);
	setCurrent(1, 0);
	setCurrent(2, 50);
	_delay_ms(100);
	Serial.begin(115200);


	//Read config from eeprom
	pulseWidth =	eeprom_read_word(&_pulseWidth);
	eeprom_read_block((void *)&cur4AllLed, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));

}

void loop()
{
	Serial.flush();
	//Определение типа команды
	if (Serial.available() > 0)
	{
		char OperationCode;
		OperationCode = Serial.read();
		switch (OperationCode)
		{
		case 'q':
		{
			factoryCalibr();
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

//go to factory calibr mode
void factoryCalibr()
{
	char pass[10];
	bool calibrEnd = false;
	uint8_t numLed=0;

	//restricted acces to factory calibration mode
	do
	{
		Serial.print(F("Enter password: \n"));
		Serial.readBytes(pass, 8);
		SerialClean();
	}
	while(strcmp(pass, "password") != 0);

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
				case 'n':  //next led choose //TODO: skip of 6-7 led combination
				{
					disableLED();
					++numLed;
					if(numLed == 21 )
						numLed = 24;
					if (numLed%4 == 0)     //light up next group of led
						shiftRegisterNext();  //
					Serial.print("LED ");
					Serial.print(numLed+1); //actually, numeration starts from zero
					Serial.print(" is on!\n");  //here we will see the classic numeration from 1
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
				case 'z':	//Go to preamp calibration
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
					SerialClean();
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
					readConfigToUart();
					break;
				}
				case 'e':	// End Calibr
				{
					readConfigToUart();
					Serial.print(F("You leave the calibration mode!\nGoodBye!"));
					calibrEnd = true;
					break;
				}
				}
				SerialClean();
			}


		//start imp
		//set curr
		//if ok save curr.
		//choose next led
	}

}

void preAmpCalibr()
{
	bool calibrEnd = false;

	Serial.print(F("You are in PreAmplifier calibration mode!\n"));
	shiftRegisterReset();

	//infinite loop for calibrating
	while (!calibrEnd)
	{

	}
}
//write to usb(uart) all data saved to eeprom in CSV format
//save it in *.csv file
//Open in excel and you will see a ordinary table
void readConfigToUart()
{
	Serial.print(F("\tThis data loaded from EEPROM.\n\n"));
	current_t cur4AllLed_temp[NUM_OF_LED];
	eeprom_read_block((void *)&cur4AllLed_temp, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
	Serial.print(F("Currents:\n"));
	Serial.print(F("LED#,I1(mA),I2(mA)\n"));
	for (uint8_t i=0;i< NUM_OF_LED; i++)
	{
		Serial.write(i);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr1);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr2);
		Serial.print(F("\n"));
	}
}

/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Функция управления потенциометрами AD5141
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
RWB1    :   установить сопротивление U3: от 0 до 100 кОм (кратно 0.39 кОм).
RWB2    :   установить сопротивление U4: от 0 до 100 кОм (кратно 0.39 кОм).
---------------------------------------------------------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Функция управления DAC
----------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Channel       :   номер канала: 1(A) или 2(B).
Напряжение    :   устанавливаемое напряжение [mV]: от 0 до 2500 mVs.
---------------------------------------------------------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/

//set width of impulse in microseconds
//step is 1us, max 200us
void setPulseWidth(uint16_t width)
{
	pulseWidth = width > MAX_PULSE_WIDTH ? MAX_PULSE_WIDTH : ( width < 0 ? 0 : width*2);
	eeprom_write_word(&_pulseWidth, pulseWidth); //save to eeprom
}

/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Включение управляющих сдвиговых регистров
----------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------*/
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
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/


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
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Выбор первой пары светодиодов
----------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------*/
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
}
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/
void disableLED()
{
	setCurrent(1, 0);		//disable led
	setCurrent(2, 0);		//
	GEN1=0;
}
/*--------------------------------------------------------------------------------------------------------------------------
----------------------------------------------------------------------------------------------------------------------------
Функция очистки serial monitor
----------------------------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------------------------*/
void SerialClean() {
	while (Serial.available() > 0)
	{
		Serial.read();
	}
}
/*--------------------------------------------------------------------------------------------------------------------------
END-----------------------------------------------------------------------------------------------------------------------*/
