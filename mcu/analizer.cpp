/**
 * Analizer.cpp
 * \author LED Microsensor
 */

#include "analizer.h"

//ISR(TIMER1_COMPA_vect)
//{
//	if (oneTimes)
//		GEN1 = 0;
//	pulseEnd= true;
//}
volatile boolean RegVal = 0;
volatile uint16_t pulseW = 816;
volatile boolean StageValMeasure = 0;
volatile uint8_t adcStop =0;
int32_t Data_ADC = 0;
int32_t Data_ADC_bgnd = 0;

ISR(TIMER1_COMPA_vect)
{
	switch(RegVal)
	{
		case 1:    /* PulseWidthMinus */
			OCR1A = 10000;                           // Change level to T-t
			TCCR1A = (1 << COM1A1) | (1 << COM1A0);  // Set mode of pins: SET (CTC-mode);

			TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10); // set CTC-mode, scale to clock = *Timer STOPPED

			RegVal = 0;
		break;

		case 0:    /* PulseWidthPlus */

			//---Timer 3----------------------------------------------------------------------------------------------------
//			StageValMeasure = 0;
//			Set_OCR3A(MeasureCycle_0_Delay);
			OCR1A = pulseW;
			adcStop =0;
			TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);   // set CTC-mode, prescaling =*1, timer START
			//---------------------------------------------------------------------------------------------------------------

//			OCR1A = pulseW;                     // Change level to t
			TCCR1A = (1 << COM1A1) | (0 << COM1A0);    // Set mode of pins: CLEAR (CTC-mode).

			RegVal = 1;
		break;
	}
}
//-------------------------------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------------------------------
ISR(TIMER0_COMPA_vect)
{
	switch(StageValMeasure)
	{
		case 0: /* STAGE CONVERTION */
		ADC_PORT |= (1 << SS_ADC);        // (OK) SS_AD7687 HIGH - Conversion start
		Set_OCR3A(5);
		TCNT0 = 0;
		StageValMeasure = 1;
		break;

		case 1: /* STAGE ACQUISITION */
		ADC_PORT = (~(1 << SS_ADC));    // (OK) SS_AD7687 LOW - Acquisition start
		if(adcStop==3) //two times measurement
			TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00); // set CTC-mode, scale to clock = *Timer STOPPED
		  // Timer clear
		adcStop > 0 ? (adcStop ==1 ? Set_OCR3A(100): Set_OCR3A(8)) : Set_OCR3A(15) ;

		TCNT0 = 0;
		if(adcStop <2) {
		Data_ADC += SPI.transfer16(0x0);
		}
		else {
			Data_ADC_bgnd += SPI.transfer16(0x0);
		}
//		Serial.println(-((Data_ADC-32767)*3000/32767));

		adcStop+=1;
		StageValMeasure = 0;

		break;
	}
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
	DDRB |=  (1 << PB5) | (1 << PB6)| (1<<WHITE_LED); //Gen_1 Gen_2 led
	PORTB &= ~((1 << PB5) | (1 << PB6) | (1 << WHITE_LED));

//	TCCR1A = (2 << COM1A0)| (3<< WGM10);  // Clear on compare match. prescaler 8,  fastpwm 10bit
//	TCCR1B = (1 << WGM12) | (2 << CS10); // T=512us=1024indixies
	TCCR1A = (0 << COM1A1) | (0 << COM1A0);  // Set mode of pins: DISCONNECTED, set CTC-mode.
	TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (0 << CS10); // set CTC-mode, scale to clock = *Timer STOPPED

//	TCCR3A =  (0 << COM1A1) | (0 << COM1A0);

	TCNT1 = 0; // Timer Clear
	GEN1 =  10;  // PreSet pulse width // 1index ==0.5us Max width is 100us=200 indicies
	TIMSK1 = (1 << OCIE1A) ; // Set interrupt on Compare A Match and Timer Overflow interrupt
	TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
	while (TCCR1B == ((1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10))) { ;}

	// ---Setup Timer2-----------------------------------------------------------------------------------------------------
		TCCR0A = (0 << COM0A1) | (0 << COM0A0) | (1<<WGM01);  // Set mode of pins: DISCONNECTED, set CTC-mode.
		TCCR0B = (0 << WGM02) | (0 << CS02) | (0 << CS01) | (0 << CS00); // set CTC-mode, scale to clock = *Timer STOPPED
		TCNT0 = 0; // Timer Clear
		TIMSK0 = (1 << OCIE0A); // Set interrupt on Compare A Match
		Set_OCR3A(10);
//		TCCR0B = (0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00);   // set CTC-mode, prescaling =*1, timer START
//		while (TCCR0B == ((0 << WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00) )) { ;}
		//---------------

	//init serial port
	Serial.begin(115200);
	_delay_ms(1500);

	//-------SS Pins ----------
	DDRF |=  (1 << SS_DAC) | (1 << SS_PREAMP) | (1 << SS_ADC);
//	DDRF |=  ;
//	DDRF |=  (1 << SS_ADC);
	PORTF |= (1 << SS_DAC) | (1 << SS_PREAMP);// | (1 << SS_ADC);
	PORTF &= ~(1<<SS_ADC);
//	PORTF |= (1 << SS_PREAMP);
//	PORTF |= (1 << SS_ADC);

	//Sample-and-Holdl-Pins
	DDRD |=  (1 << SH_SET)|(1 << SH_RESET);
	PORTD &= ~((1 << SH_SET) | (1 << SH_SET));

	//shift registr init
	DDRB |= (1<<SR_IND_ENABLE);
	DDRD |= (1 << SR_IND_CLR) | (1 << SR_IND_CLK) | (1 << SR_IND_DATA) ;
	DDRD |= (1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA) | (1<< SR_ENABLE);
	PORTD |= (1 << SR_ENABLE);      //OE HIGH - disable
	PORTB |= (1 << SR_IND_ENABLE);      //OE HIGH - disable
	PORTD &= ~((1 << SR_CLR) | (1 << SR_CLK) | (1 << SR_DATA));   //set  SRCLR / RCLK / SER -LOW
	PORTD &= ~((1 << SR_IND_CLR) | (1 << SR_IND_CLK) | (1 << SR_IND_DATA));   //set  SRCLR / RCLK / SER -LOW

	//btn init
	DDRE &= ~(1<< BTN);
	PORTE |= (1<<BTN);
	//initialize SPI
	SPI.setBitOrder(MSBFIRST);
	SPI.setDataMode(SPI_MODE2);
	SPI.begin();

	Serial.print(F("Reading config... "));
//	pulseWidth =	eeprom_read_word(&_pulseWidth);
	eeprom_read_block((void *)&cur4AllLed, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
	eeprom_read_block((void *)&coeffs, (const void *) &_coefficients, NUM_OF_LED*sizeof(float));
	Serial.print(F("Done!\r\n\n"));
	writeConfigToUart();

	shiftRegisterReset();
//	shiftRegisterFirst();
	PORTB &= (~(1 << SR_IND_ENABLE));
	indicatorBlinc();

	PORTB |= (1<<WHITE_LED);
	PORTD &= (~(1 << SR_ENABLE));  //OE HIGH - enble shiftreg
	doOnePulse(10);
}

/**
 * \defgroup main_menu Main menu
 * @{
 */
void loop()
{
	Serial.flush();
//	_delay_ms(200);			//debug
//	singleLEDBlinc();
///Button way
	if ( (PINE & ( 1 << BTN )) == 0 )
	{
		_delay_ms(150); //filtering
		if ( (PINE & ( 1 << BTN )) == 0 )
		{
			Serial.print(F("x=m\n")); //send command
			while( (PINE & ( 1 << BTN )) == 0 ) { _delay_us(1);} //wait unpressed
		}
	}


	/// Available in main menu commands:
	if (Serial.available() > 0)
	{
		char OperationCode;

		OperationCode = Serial.read();
		switch (OperationCode)
		{
			case 'z':
					{
						Serial.print(F("reset\r\n"));
						shiftRegisterReset();
						break;
					}
			case 'x':
					{
						Serial.print(F("next\r\n"));
						shiftRegisterNext();
						break;
					}
			case 'c':
					{
						Serial.print(F("first\r\n"));
						shiftRegisterFirst();
						break;
					}
			case 'v':
					{
						Serial.print(F("pulse\r\nfirst\r\n"));
						setPreAmp(20,100);
//						shiftRegisterReset();
//						shiftRegisterFirst();
//						_delay_ms(150);
//						for(uint8_t i=0;i<6;i++){
//							shiftRegisterNext();
//						}
						for(uint8_t j=0; j<5; j++)
						{
							setCurrent(1, 50);
							setCurrent(2, 0);
							_delay_us(PULSE_DELAY);
							doOnePulse(120);
							_delay_ms(200);
						}
						disableLED();
							break;
					}
			case 'b':
					{
						Serial.print(F("blinctest\r\n"));
						indicatorBlinc();
						break;
					}
			case 'q':
			{
				/// - 	\b q \n
				/// Go to factory calibration function.
				factoryCalibr();
				break;
			}
			case 'e':
			{
				/// - \b e \n
				/// Express calibration method. Not implemented yet
				Serial.print(F("Not implemented yet \n"));
				break;
			}
			case 's':
			{
				/// - 	\b s \n
				/// One mode of measurements with option of coeffs calibration \n
				/// Manually choosing preamps parameters \n
				/// Sample&Holde mode
				doMeasurementsSH(0, false);
				break;
			}
			case 'm':
			{
				/// - 	\b m \n
				/// One mode of measurements with option of coeffs calibration \n
				/// Manually choosing preamps parameters \n
				/// Inpulse measurements mode
				doMeasurements(0, false);
				break;
			}
			case 'a':
			{
				/// - 	\b a \n
				/// One mode of measurements with option of coeffs calibration \n
				/// Series of measurements with predefined preamp parameters \n
				/// Sample&Holde mode
				doMeasurementsSH_Avg();
				break;
			}
			default:
			{
				Serial.print(OperationCode);
				SerialClean();
				break;
			}
		}
		SerialClean();
	}
}
///@}

/**
 *  \defgroup factoryCalibr Calibration mode
 * @{
 */
void factoryCalibr()
{
	bool calibrEnd = false;
	uint8_t numLed=0;
	SerialClean();
	Serial.print(F("You are in calibration mode!\r\n"));
	shiftRegisterReset();
	uint8_t numEtalon=0;
	etalon_t etalons[NUM_OF_ETALON];
	//infinite loop for calibrating
	while (!calibrEnd)
	{
		Serial.flush();
		/// Available in calibration menu commands:
		if (Serial.available() > 0)
		{
			char OperationCode;
			OperationCode = Serial.read();
			switch (OperationCode)
			{
				case 'w':
				{
					/// - 	\b w \n
					/// Set width of impulse in microseconds\n step is 1us, max 200us
					uint16_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					setPulseWidth(curValue);
					Serial.print(F("Set pulse width to: "));
					Serial.println(curValue);
					break;
				}
				case 't':
				{
					/// - 	\b t \n
					/// Save to eeprom value of input resistanse of second OpAmp
					float curValue = 0;
					curValue = Serial.parseFloat(SKIP_WHITESPACE);
					setC_R(curValue);
					Serial.print(F("Set input resistanse to: "));
					Serial.println(curValue);
					break;
				}
				case 'a':
				{
					/// - 	\b a \n
					/// Set currents for LED in channel \b 1\n
					/// Max value is 1000mA, with step 1mA
					uint8_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					cur4AllLed[numLed].curr1 = curValue;
					setCurrent(1, curValue);
					break;
				}
				case 'b':
				{
					/// - 	\b b \n
					/// Set currents for LED in channel \b 2\n
					/// Max value is 1000mA, with step 1mA
					uint8_t curValue = 0;
					curValue = Serial.parseInt(SKIP_WHITESPACE);
					cur4AllLed[numLed].curr2 = curValue;
					setCurrent(2, curValue);
					break;
				}
				case 'g':
				{
					/// - 	\b g \n
					/// reset LEDs if something wrong\n
					disableLED();
					numLed=0;
					shiftRegisterReset();
					Serial.print(F("All LEDs is off.\r\nNow you can restart calibration.\r\n"));
					break;
				}
				case 'f':
				{
					/// - 	\b f \n
					/// Light-up first led\n
					disableLED();
					numLed=0;
					shiftRegisterFirst();
					Serial.print(F("First led is on!\r\n"));
					break;
				}
				case 'n':
				{
					/// - 	\b n \n
					/// Next led choose\n
					disableLED();
					if (numLed == NUM_OF_LED-1)
					{ //TODO: do correct exit, return to first led or just end.
						Serial.print("LED ");
						Serial.print(numLed+1); //actually, numeration starts from zero
						Serial.print(" still is on!\r\n");  //here we will see the classic numeration from 1
						Serial.print(F("This is end of calibration\r\n"));
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
						Serial.print(" is on!\r\n");  //here we will see the classic numeration from 1
					}
					break;
				}
				case 'o':
				{
					/// - 	\b o \n
					/// Start infinite series of pulse to LED\n
					for(uint8_t i=0; i<10; ++i)
					{
						doOnePulse(pulseW);
						Serial.println(( ( Data_ADC >> 1) + ( 0xFFFF - ( Data_ADC_bgnd >> 1 ) ) ) * 3300.0 / 32767.0);
						Data_ADC=0;
						Data_ADC_bgnd =0;
						_delay_ms(150);
					}
					break;
				}
				case 'p':
				{
					/// - 	\b p \n
					/// Make one pulse to LED\n
					doOnePulse(pulseW);
					Serial.println(( ( Data_ADC >> 1) + ( 0xFFFF - ( Data_ADC_bgnd >> 1 ) ) ) * 3300.0 / 32767.0);
					Data_ADC=0;
					Data_ADC_bgnd =0;
					break;
				}
				case 'i':
				{
					/// - 	\b i \n
					/// Disable LED\n
					disableLED();
					break;
				}
				case 'z':
				{
					/// - 	\b z \n
					/// Return current led number\n
					Serial.print(F("Current LED is "));
					Serial.println(numLed+1);
					break;
				}
				case 'c':
				{
					/// - 	\b c \n
					/// Set coeffs of OpAmp\n
					/// Control digital potentiometers ad5141\n
					/// Max value is 100kOhm, with step 0.39kOhm\n
					uint8_t n=1; //number of led group
					float resVal1;
					float resVal2;
					n = Serial.parseInt(SKIP_WHITESPACE);
					resVal1 = Serial.parseFloat(SKIP_WHITESPACE);
					resVal2 = Serial.parseFloat(SKIP_WHITESPACE);
					setPreAmp(resVal1,resVal2);
					Serial.println(n);
					resVal1 = (resVal1 > 100.0) ? 100.0 : ((resVal1 < 0) ? 0 : resVal1);
					resVal2 = (resVal2 > 100.0) ? 100.0 : ((resVal2 < 0.0) ? 0.0 : resVal2);
					if (n==1) {
						etalons[numEtalon].g1_1 = resVal1;
						etalons[numEtalon].g2_1 = resVal2;
					}
					else {
						etalons[numEtalon].g1_2 = resVal1;
						etalons[numEtalon].g2_2 = resVal2;
					}
					break;
				}
				case 'r':
				{
					/// - 	\b r \n
					/// Go to preAmp calibration \n
					preAmpCalibr();
					break;
				}
				case 's':
				{
					/// - 	\b s \n
					/// Save parameters for all LED to EEPROM\n
					eeprom_write_block((const void *)&cur4AllLed, (void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
					eeprom_write_block((const void *)&etalons, (void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
//					writeConfigToUart();
					break;
				}
				case 'e':	// End Calibr
				{
					/// - 	\b e \n
					/// Exit from calibration mode\n
					writeConfigToUart();
					Serial.print(F("You leave the calibration mode!\nGoodBye!"));
					calibrEnd = true;
					break;
				}
				case 'd': //random init
				{
					etalon_t etalons[NUM_OF_ETALON];
					setPulseWidth(40);
					setC_R(3.9);
					for (uint8_t i=0;i< NUM_OF_LED; i++)
					{
						if(i==21) {i=24;}
						cur4AllLed[i].curr1 =80+i;
						cur4AllLed[i].curr2 =80+i;
						coeffs[i] = 1.1;
					}
					for (uint8_t k=0;k<NUM_OF_ETALON; k++)
					{
						etalons[k].g1_1 = 100;
						etalons[k].g2_1 = 100;
						etalons[k].g2_2 = 100;
						etalons[k].g1_2 = 20;
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
///@}


/**
 *  \defgroup preAmpCalibr Etalon's data write
 * @{
 */
void preAmpCalibr()
{
	bool calibrEnd = false;
	uint8_t numEtalon=0;
	etalon_t etalons[NUM_OF_ETALON];
	Serial.print(F("You are in PreAmplifier calibration mode!\r\n"));
	shiftRegisterReset();
	//infinite loop for calibrating
	while (!calibrEnd)
	{
		Serial.flush();
		//Определение типа команды
		/// Available in PreAmp calibration menu commands:
		if (Serial.available() > 0)
		{
			char OperationCode;
			OperationCode = Serial.read();
			switch (OperationCode)
			{
				case 'c':
				{
					/// - 	\b c \n
					/// Set coeffs of OpAmp\n
					/// Control digital potentiometers ad5141\n
					/// Max value is 100kOhm, with step 0.39kOhm\n
					float resVal1;
					float resVal2;
					resVal1 = Serial.parseFloat(SKIP_WHITESPACE);
					resVal2 = Serial.parseFloat(SKIP_WHITESPACE);
					setPreAmp(resVal1,resVal2);
					resVal1 = (resVal1 > 100.0) ? 100.0 : ((resVal1 < 0) ? 0 : resVal1);
					resVal2 = (resVal2 > 100.0) ? 100.0 : ((resVal2 < 0.0) ? 0.0 : resVal2);
					etalons[numEtalon].g1_1 = resVal1;
					etalons[numEtalon].g2_1 = resVal2;
					break;
				}
				case 'x':
				{
					/// - 	\b x \n
					/// Choose etalon cell in memory\n
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
				case 'e':
				{
					/// - 	\b e \n
					/// Exit from calibration mode\n
					writeConfigToUart();
					Serial.print(F("You leave the PreAmplifier calibration mode!\nGoodBye!"));
					calibrEnd = true;
					break;
				}
				case 's':
				{
					/// - 	\b s \n
					/// Save parameters  to EEPROM\n
					eeprom_write_block((const void *)&etalons, (void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
					writeConfigToUart();
					break;
				}
				case 'z':
				{
					/// - 	\b z \n
					/// Return current etalon cell number\n
					Serial.print(F("Current etalon is "));
					Serial.println(numEtalon+1);
					break;
				}
				case 'k':
				{
					/// - 	\b k \n
					/// Save k for etalon\n
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
				case 'm':
				{
					/// - 	\b m \n
					/// set min and max value of g2\n
					float curVal;
					curVal = Serial.parseFloat(SKIP_WHITESPACE);
					etalons[numEtalon].g1_2 = curVal;
					Serial.print(F("Min is "));
					Serial.println(curVal);
					curVal = Serial.parseFloat(SKIP_WHITESPACE);
					etalons[numEtalon].g2_2 = curVal;
					Serial.print(F("Max is "));
					Serial.println(curVal);
					break;
				}
				case 'n':
				{
					/// - 	\b n \n
					/// Compute Ai\n
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
				case 'a':
				{
					/// - 	\b a \n
					/// Compute Ai with doMeasurementsSH_Avg method\n
					float coeff_temp[NUM_OF_LED];
					eeprom_read_block((void *)coeff_temp, (const void *)_coefficients, NUM_OF_LED*sizeof(float));
					doMeasurementsSH_Avg(true);
					Serial.print(F("Ai#, Previous , Current, Diff(%)\r\n"));
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
						Serial.print(F("\r\n"));
					}
					Serial.print(F("If params is ok - press \"y\".\nFor reset press any key.\r\n"));
					while(Serial.available()==0){_delay_ms(1);} //wait for command
					if (Serial.read() == 'y')
						eeprom_write_block((const void *)coeffs, (void *)_coefficients, NUM_OF_LED*sizeof(float)); //save params from last measurement
					else
						eeprom_read_block((void *)coeffs, (const void *)_coefficients, NUM_OF_LED*sizeof(float)); //reset params in coeffs
					break;
				}
				case 'l':
				{
					/// - 	\b l \n
					/// Write Ai in manual mode\n
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
///@}

void doMeasurements(uint8_t numOfEtalon, bool calcNorm)
{
	float measuredU[NUM_OF_LED];
	float k=0; // norm coeefs or result
	uint8_t index;
//	Serial.print(F("You are in measurements mode!\r\n"));
	if (!calcNorm)
	{
//		do {
//			Serial.print(F("Set etalon # \n"));
//			while (Serial.available() == 0) {_delay_ms(1);}
//			index = Serial.parseInt(SKIP_WHITESPACE);
//		}
//		while (index > NUM_OF_ETALON && index < 1);
//		--index; //convert to programmers numering
		index = 0;
	}
	else
		index = numOfEtalon;
	etalon_t etalonForCalc;
	float c_R = eeprom_read_float(&_c_R); //input resistanse of g2
	uint16_t pulseWidth =	eeprom_read_word(&_pulseWidth);
	eeprom_read_block((void *)&etalonForCalc, (const void *)&_etalons[index], sizeof(etalon_t));
	float constant = calcNorm ? (etalonForCalc.g1_1 * etalonForCalc.g2_1 / c_R) :(etalonForCalc.g1_1 * etalonForCalc.g2_1/c_R) ;
	//measurements
	shiftRegisterReset();
//	disableLED();
	shiftRegisterFirst(); //select first led pair

	PORTB &= ~(1<<WHITE_LED);
	indicatorBlinc(); //some waiting before measurements

	setPreAmp(etalonForCalc.g1_1, etalonForCalc.g2_1);
	//TODO:
	for (uint8_t i = 0; i< NUM_OF_LED; i++)
	{
		if( i == 21 ) i = 24;//skip 6-7 leds combinations
		if( i != 0 && i % 4 == 0) // !=0 for first led exception
			{
				setPreAmp(0,0);
				shiftRegisterNext();
				_delay_ms(10);
				if ( i < 24)
					setPreAmp(etalonForCalc.g1_1, etalonForCalc.g2_1);
				else
					setPreAmp(etalonForCalc.g1_2, etalonForCalc.g2_2);
			}

		setCurrent(1, cur4AllLed[i].curr1);
		setCurrent(2, cur4AllLed[i].curr2);

		doOnePulse(pulseWidth);
//		_delay_ms(50);

//		Serial.print(F("v: "));
//		Serial.println(((Data_ADC>>1)));
//		Serial.print(F("b: "));
//		Serial.println(Data_ADC_bgnd>>1);

		measuredU[i] = ( ( Data_ADC >> 1) + ( 0xFFFF - ( Data_ADC_bgnd >> 1 ) ) ) * 3300.0 / 32767.0;

		Data_ADC=0;
		Data_ADC_bgnd =0;

//		while(!pulseEnd) {_delay_us(1);}
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
//		Serial.print(F("Coefficients are ready to save.\r\n"));
	}
	else //calc k of sample
	{
//		Serial.print(F("LED#,k,Uamp(mV)\r\n"));
		for (uint8_t i = 0; i< NUM_OF_LED; i++)
		{
			if(i == 21 ) //skip 6-7 leds combinations
				i = 24;
			Serial.print(F("x=d,"));
			Serial.print(i);
			Serial.print(F(","));
			k = measuredU[i] / ( coeffs[i] * constant);
			Serial.print(k);
			Serial.print(F(","));
			Serial.println(measuredU[i]);
		}
		Serial.print(F("x=e\n")); //end transmit
	}
	indicatorBlinc();
	PORTB |= (1<<WHITE_LED);
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
	Serial.print(F("You are in measurements mode!\r\n"));
	if (!calcNorm)
	{
		do {
			Serial.print(F("Set etalon # \r\n"));
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
	uint16_t pulseWidth =	eeprom_read_word(&_pulseWidth);
	float constant = calcNorm ? (etalonForCalc.g1_1 * etalonForCalc.g2_1 / c_R) :(etalonForCalc.g1_1 * etalonForCalc.g2_1/c_R) ;

	setPreAmp(etalonForCalc.g1_1, etalonForCalc.g2_1);
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
		doOnePulse(pulseWidth);
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
		Serial.print(F("Coefficients are ready to save.\r\n"));
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
	Serial.print(F("You are in measurements mode!\r\n"));

	float kAvg[NUM_OF_LED] = {0.0};
	float c_R = eeprom_read_float(&_c_R); //input resistanse of g2
	uint16_t pulseWidth =	eeprom_read_word(&_pulseWidth);

	for(uint8_t index = 0; index < NUM_OF_ETALON; index++)
	{
		float k=0; // norm coeefs or result
		float measuredU[NUM_OF_LED] ;
		float value = 0;
		float background=0;
		//read config
		etalon_t etalonForCalc;
		eeprom_read_block((void *)&etalonForCalc, (const void *)&_etalons[index], sizeof(etalon_t));
		float constant = etalonForCalc.g1_1 * etalonForCalc.g2_1 / c_R ;
		setPreAmp(etalonForCalc.g1_1, etalonForCalc.g2_1);
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
			doOnePulse(pulseWidth);
			while(!pulseEnd) {_delay_us(1);}
			readADC(value); //convert to mV
			measuredU[i] = value-background;
			disableLED();
		}
		if (calcNorm) // part of express calibration
			Serial.print(F("LED#,Ai,Uamp(mV)\r\n"));
		else
			Serial.print(F("LED#,k,Uamp(mV)\r\n"));
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

	Serial.print(F("\r\nAverage values.\r\n"));
	if (calcNorm) // part of express calibration
		Serial.print(F("LED#,Ai,\r\n"));
	else
		Serial.print(F("LED#,k\r\n"));
//calc and print average value
	for(uint8_t i = 0; i< NUM_OF_LED; i++)
	{
		if( i == 21 ) i = 24;
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
//TODO:
void readADC(float& value)
{
	int val=0;
	for (uint8_t i=0;i<1;i++)
	{
		int temp=0;
		ADC_PORT |= (1 << SS_ADC); //start conversion
		_delay_us(3);
		ADC_PORT &= ~(1<< SS_ADC); // set to low for start acquisition
		_delay_us(1);
		temp = SPI.transfer16(0x0);
		val += temp;
	}
//	val = val>>1; // devide by 2
	value = val;//* REFERENCE_V / 32767.0; //convert to mV
}

//write to usb(uart) all data saved to eeprom in CSV format
//save it in *.csv file
//Open in excel and you will see a ordinary table
void writeConfigToUart()
{
	Serial.print(F("data from EEPROM.\r\n"));
	//pulse
	uint16_t pulse_temp = eeprom_read_word(&_pulseWidth);
	Serial.print(F("\r\nPWidth is: "));
	Serial.println(pulse_temp/2); //because PW in memory in ticks, not in us
	//resistance
	float res_temp = eeprom_read_float(&_c_R);
	Serial.print(F("\r\nInRes: "));
	Serial.println(res_temp, 3);
	//current
	current_t cur4AllLed_temp[NUM_OF_LED];
	eeprom_read_block((void *)&cur4AllLed_temp, (const void *) &_pairsOfCurrent, NUM_OF_LED*sizeof(current_t));
	Serial.print(F("\r\nCurrents:\r\n"));
	Serial.print(F("LED#, I1(mA), I2(mA)\r\n"));
	for (uint8_t i=0;i< NUM_OF_LED; i++)
	{
		if(i==21) {i=24;}
		Serial.print(i+1);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr1);
		Serial.print(F(","));
		Serial.print(cur4AllLed_temp[i].curr2);
		Serial.print(F("\r\n"));
	}
	//preamp and etalon
	etalon_t etalons_temp[NUM_OF_ETALON];
	eeprom_read_block((void *)&etalons_temp, (const void *) &_etalons, NUM_OF_ETALON*sizeof(etalon_t));
	Serial.print(F("\r\nPreamp Data:\r\n"));
	Serial.println(F("Etalon#, g1_1, g2_1, g2_2 g2_2"));
	for (uint8_t i=0;i< NUM_OF_ETALON; i++)
	{
		Serial.print(i+1);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g1_1);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g2_1);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g1_2);
		Serial.print(F(","));
		Serial.print(etalons_temp[i].g2_2);
//		for(uint8_t k=0; k<NUM_OF_LED;k++)
//		{
//			if(k==21) {k=24;}
//			Serial.print(F(","));
//			Serial.print(etalons_temp[i].k[k]);
//		}
		Serial.print(F("\r\n"));
	}
	//coeffs
//	float coeffs_temp[NUM_OF_LED];
//	eeprom_read_block((void *)&coeffs_temp, (const void *) &_coefficients, NUM_OF_LED*sizeof(float));
//	Serial.print(F("\r\nCoeefs Ai:\r\n"));
//	Serial.print(F("LED#, Ai\r\n"));
//
//	for (uint8_t i=0;i< NUM_OF_LED; i++)
//	{
//		if(i==21) i=24;
//		Serial.print(i+1);
//		Serial.print(F(","));
//		Serial.print(coeffs_temp[i]);
//		Serial.print(F("\r\n"));
//	}
//	Serial.print(F("This data loaded from EEPROM.\r\n"));
}

void setC_R(float val)
{
	eeprom_write_float(&_c_R, val); //save to eeprom
}

void doOnePulse(uint16_t pulseWidth)
{
//	oneTimes = true;
//	pulseEnd = false;
//	TCNT1 = 0;
	GEN1=pulseW;
	TCNT1 = 65530;
//	float value;
//	readADC(value);
	TCCR1A = (1 << COM1A1) | (1 << COM1A0);
	TCCR1B = (1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10);
	while (TCCR1B == ((1 << WGM12) | (0 << CS12) | (0 << CS11) | (1 << CS10)))  {}
	while (TCCR0B == ((0<< WGM02) | (0 << CS02) | (1 << CS01) | (0 << CS00))) {}
//
//	Serial.print(F("v: "));
//	Serial.println(((Data_ADC>>1))&0x0FFF);
//	Serial.print(F("b: "));
//	Serial.println(0xFFFF-(( Data_ADC_bgnd>>1)));

//	Data_ADC=0;
//	Data_ADC_bgnd =0;
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


//Включение управляющих сдвиговых регистров
//set register to 0xFFFF
void indicatorRegisterReset() {
	//reset register
	PORTD &= (~(1 << SR_IND_CLR));   //SRCLR-CLEAR-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_IND_CLK);      //RCLK-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_IND_CLK));   //RCLK-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_IND_CLR);      //SRCLR-CLEAR-HIGH;

	PORTD |= (1 << SR_IND_DATA);      //SER-HIGH
	for (uint8_t i = 0; i < 14; i++)
	{
		PORTD |= (1 << SR_IND_CLK);      //RCLK-HIGH
		_delay_us(ShiftRegisterDelay);
		PORTD &= (~(1 << SR_IND_CLK));   //RCLK-LOW
		_delay_us(ShiftRegisterDelay);
	}
	PORTD &= (~(1 << SR_IND_DATA));   //SER-LOW
	_delay_us(ShiftRegisterDelay);
}

/*
 * Выбор следующей пары светодиодов
 */
void indicatorRegisterNext() {
	PORTD |= (1 << SR_IND_DATA);      //SER-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD |= (1 << SR_IND_CLK);      //RCLK-HIGH
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_IND_CLK));   //RCLK-LOW
	_delay_us(ShiftRegisterDelay);
	PORTD &= (~(1 << SR_IND_DATA));   //SER-LOW
}

//Выбор первой пары светодиодов
void indicatorRegisterFirst() {
	PORTD &= (~(1 << SR_IND_DATA));   //SER-LOW
	_delay_us(ShiftRegisterDelay);
	for (uint8_t i = 0; i < 2; i++)
	{
		PORTD |= (1 << SR_IND_CLK);      //RCLK-HIGH
		_delay_us(ShiftRegisterDelay);
		PORTD &= (~(1 << SR_IND_CLK));   //RCLK-LOW
		_delay_us(ShiftRegisterDelay);
	}
}

void indicatorBlinc()
{
	indicatorRegisterReset();
	indicatorRegisterFirst();
	for (uint8_t i=0;i<17;i++)
	{
		_delay_ms(30);
		indicatorRegisterNext();
	}
}

void singleLEDBlinc()
{
	for (uint8_t i =0;i<2;i++)
	{
		PORTB |= (1<<WHITE_LED);
		_delay_ms(50);
		PORTB &= ~(1<<WHITE_LED);
		_delay_ms(100);
	}
}

void disableLED()
{
//	GEN1=0;
	setCurrent(1, 0);		//disable led
	setCurrent(2, 0);		//
}

//Функция очистки serial monitor
void SerialClean() {
	while (Serial.available() > 0)
	{
		Serial.read();
	}
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
	cli();

	PREAMP_PORT &= (~(1 << SS_PREAMP));  //SS_AD5141 LOW
	_delay_us(WBDelay);
	SPI.transfer(B00010000);    //Write to RDAC
	SPI.transfer(RWB2_code);
	SPI.transfer(B00010000);    //Write to RDAC
	SPI.transfer(RWB1_code);

	_delay_us(WBDelay);

	//read data from resistance
//	SPI.transfer(B00110000);    //Read from RDAC
//	RWB2_code = SPI.transfer(B00000011); //second
//	SPI.transfer(B00110000);    //Read from RDAC
//	RWB1_code = SPI.transfer(B00000011); //first
//	_delay_us(WBDelay);
	PREAMP_PORT |= (1 << SS_PREAMP);     //SS_AD5141 HIGH
	_delay_us(WBDelay);
//	Serial.print(F("r1: "));
//	Serial.print((RWB1_code*100/255.0),1);
//	Serial.print(F(", r2: "));
//	Serial.println((RWB2_code*100/255.0),1);

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
	Serial.print(F("c"));
	Serial.print(channelN);
	Serial.print(F(": "));
	Serial.println(savedValue*2500.0/65536, 1);

	DAC_PORT |= (1 << SS_DAC);      //SS_AD5689 HIGH
}

//set width of impulse in microseconds
//step is 1us, max 200us
void setPulseWidth(uint16_t width)
{
	width = width > MAX_PULSE_WIDTH ? MAX_PULSE_WIDTH : ( width < 0 ? 0 : width*2);
	eeprom_write_word(&_pulseWidth, width); //save to eeprom
}
