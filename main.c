/*
* ----------------------------------------------------------------------------
Created by Rotating Fans 11/12/16
Licensed under GPLv3
* -----------------------------------------------------------------------------
*/

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#define ECB 0
#define MULTIPLY_AS_A_FUNCTION 1
#include "./AESCBC/aes.h"

//#include <stdint.h>
#include "./nrf24.h"
//#include <util/delay.h>

// Value when sum of ADC values is more than first value in table
#define TEMPERATURE_UNDER -400
// Value when sum of ADC values is less than last value in table
#define TEMPERATURE_OVER 500
// Value corresponds to first entry in table
#define TEMPERATURE_TABLE_START -400
// Table step
#define TEMPERATURE_TABLE_STEP 10
#define adcValues 91
// Type of each table item. If sum fits into 16 bits - uint16_t, else - uint32_t
typedef uint16_t temperature_table_entry_type;
// Type of table index. If table has more than 255 items, then uint16_t, else - uint8_t
typedef uint8_t temperature_table_index_type;
// Access method to table entry. Should correspond to temperature_table_entry_type

// Data logging configuration.
#define LOGGING_FREQ_SECONDS   8       // Seconds to wait before a new sensor reading is logged.
// Logging server listening port.
#define MAX_SLEEP_ITERATIONS   LOGGING_FREQ_SECONDS / 8  // Number of times to sleep (for 8 seconds) before
// a sensor reading is taken and sent to the server.
// Don't change this unless you also change the
// watchdog timer configuration.
/* Table of ADC sum value, corresponding to temperature. Starting from higher value to lower.
NTC R25=10k
R1=10k
R3=7.68k
VRange=3-2
AREF=VCC
TempRange=-40-50
ADC Value Accuracy - +- 3 * 32
Circuit:
VCC -- R1 -- | -- NTC -- R3 -- GND
Vout
*/
const temperature_table_entry_type termo_table[] PROGMEM = {
	31184,31088,31024,30928,30832,30736,30624,30528,30432,30320,
	30208,30080,29968,29840,29712,29584,29456,29296,29168,29008,
	28880,28720,28560,28416,28240,28080,27920,27744,27584,27408,
	27232,27040,26880,26688,26496,26304,26144,25952,25760,25568,
	25376,25184,24992,24800,24608,24416,24224,24032,23840,23648,
	23472,23280,23104,22912,22736,22560,22368,22192,22032,21840,
	21680,21504,21344,21168,21008,20848,20688,20528,20368,20224,
	20080,19936,19776,19648,19488,19360,19232,19104,18976,18848,
	18720,18608,18480,18368,18256,18144,18048,17936,17824,17728,17632};
/* ------------------------------------------------------------------------- */
//uint8_t q = 0;
//uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
volatile uint8_t watchdogActivated = 1;
//char tempstr[10] = "";
//int16_t tempC = 0;

uint8_t EEMEM paired = 0;
uint8_t EEMEM encryptionKey[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
uint8_t EEMEM rxAddr[5] = {0xE7,0xE7,0xE7,0xE7,0xE6};
uint8_t EEMEM txAddr[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
char currentIv[16] ={};
char nextIv[16] = {};
static void sendData(uint8_t *keyVal, char *id);
volatile uint8_t seconds;
/* ------------------------------------------------------------------------- */
ISR(WDT_vect)
{
	// Set the watchdog activated flag.
	// Note that you shouldn't do much work inside an interrupt handler.
	watchdogActivated = 1;
}
ISR(TIMER1_COMPA_vect)
{
	++seconds;
}
uint16_t TEMPERATURE_TABLE_READ(uint8_t i) {pgm_read_byte(&termo_table[i]);}
void sleep()
{
	// Set sleep to full power down.  Only external interrupts or
	// the watchdog timer can wake the CPU!
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);

	// Disable the ADC while asleep.
	power_adc_disable();

	// Enable sleep.
	sleep_enable();
	
	// Disable brown-out detection during sleep.  This is timing critical and
	// must be done right before entering sleep mode.
	MCUCR |= (1<<BODS) | (1<<BODSE);
	MCUCR &= ~(1<<BODSE);
	
	// Enter sleep mode.
	sleep_cpu();

	// CPU is now asleep and program execution completely halts!
	// Once awake, execution will resume at this point.
	
	// When awake, disable sleep mode and turn on all devices.
	sleep_disable();
	power_all_enable();
}
void adc_init()
{

}
int main()
{
	uint8_t keyVal[16];
	uint8_t rxAdr[5];
	uint8_t txAdr[5];
	uint8_t pairedVal;
	uint8_t sleepIterations = 0;

	/* init hardware pins */
		eeprom_read_block(txAddr,txAddr,5);

		eeprom_read_block(rxAddr,rxAddr,5);
	nrf24_init(2,16,rxAdr,txAddr);
	
	/* Channel #2 , payload length: 32 */

	// AREF = AVcc
	ADMUX = (1<<REFS0);
	
	// ADC Enable and prescaler of 128
	// 16000000/128 = 125000
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);	//
	///* Set the device addresses */




	DDRB = 0xFF;
	pairedVal = eeprom_read_byte(paired);
	if (pairedVal) {	
		pingHost();
	}
	else {
		pair();
	}
	eeprom_read_block((void*)&keyVal, (const void*)&encryptionKey, 16);
	//aes128_init(keyVal,&ctx);
	//aes128_enc(data,&ctx);
	// Setup the watchdog timer to run an interrupt which
	// wakes the AVR from sleep every 8 seconds.
	
	// Note that the default behavior of resetting the AVR
	// with the watchdog will be disabled.
	
	// This next section of code is timing critical, so interrupts are disabled.
	// See more details of how to change the watchdog in the ATmega328P datasheet
	// around page 50, Watchdog Timer.
	cli();
	
	// Set the watchdog reset bit in the MCU status register to 0.
	MCUSR &= ~(1<<WDRF);
	
	// Set WDCE and WDE bits in the watchdog control register.
	WDTCSR |= (1<<WDCE) | (1<<WDE);

	// Set watchdog clock prescaler bits to a value of 8 seconds.
	WDTCSR = (1<<WDP0) | (1<<WDP3);
	
	// Enable watchdog as interrupt only (no reset).
	WDTCSR |= (1<<WDIE);
	TCCR1A = 0;     // set entire TCCR1A register to 0
	TCCR1B = 0;     // same for TCCR1B
	
	// set compare match register to desired timer count:
	OCR1A = 15624;
	// turn on CTC mode:
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS12 bits for 1024 prescaler:
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << CS12);
	// enable timer compare interrupt:
	TIMSK1 |= (1 << OCIE1A);
	// Enable interrupts again.
	sei();



	for(;;)	{
		if (watchdogActivated)
		{
			watchdogActivated = 0;
			
			// Increase the count of sleep iterations and take a sensor
			// reading once the max number of iterations has been hit.
			if (++sleepIterations >= MAX_SLEEP_ITERATIONS) {
				// Reset the number of sleep iterations.
				sleepIterations = 0;
				// Log the sensor data (waking the CC3000, etc. as needed)
				sendData(keyVal, (char) {1,0,0});
			}
		}
		
		// Go to sleep!
		sleep();
		
	}
}
static void sendData(uint8_t *keyVal, char *id) {

	char Payload[16] = "";
	char tempstr[6] = "";
	int16_t tempC = 0;
	uint16_t summ = 0;
	PORTB = (1<<0);
	uint8_t i = 32;
	// select the corresponding channel 0~7
	// ANDing with ’7′ will always keep the value
	// of ‘ch’ between 0 and 7
	// AND operation with 7
	ADMUX = (ADMUX & 0xF8)|1; // clears the bottom 3 bits before ORing
	do {

		ADCSRA |= _BV(ADSC);
		while(ADCSRA & (1<<ADSC));
		summ += ADC;
	} while (--i);


	PORTB = (0<<0);
	tempC = calc_temperature(summ);

	itoa(tempC,tempstr,10);
	AES128_CBC_encrypt_buffer(tempstr,tempstr,6,keyVal,currentIv);
	strcat(Payload, id);
	strcat(Payload,tempstr);


	/* Automatically goes to TX mode */
	nrf24_send(Payload);

	/* Wait for transmission to end */
	nrf24_powerUpRx();
	seconds = 0;
	while (!nrf24_dataReady() && seconds < 2);
	if (nrf24_dataReady()) {
		nrf24_getData(Payload);
		AES128_CBC_decrypt_buffer(Payload,Payload,16,keyVal,currentIv);
		strcat(Payload,id);
		strcat(Payload, "ACK");
		nrf24_send(Payload);
	}

	///* Make analysis on last transmission attempt */
	//temp = nrf24_lastMessageStatus();
	///* Retransmission count indicates the transmission quality */
	//temp = nrf24_retransmissionCount();

	/* Or you might want to power down after TX */
	nrf24_powerDown();

}
void pair() {
	//Broadcast Pair request
	//Listen for Ack with tx Addr
	//Ack and requst AEs key, encrypt w/ Comm key
	//Listen for unique key
	//Ack uniqe key
	//Send Get IV Command
	//read IV
	//ACk cur IV, Request NExt
	//Read next
	//ACK

}

void pingHost() {
	//Ask to be paired
	//If Yes do Pair(), set paired to 0
	//Else:
	//Send Get IV Command
	//read IV
	//ACk cur IV, Request NExt
	//Read next
	//ACK

}
// This function is calculating temperature in tenth of degree of Celsius
// depending on ADC sum value as input parameter.
int16_t calc_temperature(temperature_table_entry_type adcsum) {
	temperature_table_index_type l = 0;
	temperature_table_index_type r = adcValues;
	temperature_table_entry_type thigh = TEMPERATURE_TABLE_READ(r);
	
	// Checking for bound values
	if (adcsum <= thigh) {
		#ifdef TEMPERATURE_UNDER
		if (adcsum < thigh)
		return TEMPERATURE_UNDER;
		#endif
		return TEMPERATURE_TABLE_STEP * r + TEMPERATURE_TABLE_START;
	}
	temperature_table_entry_type tlow = TEMPERATURE_TABLE_READ(0);
	if (adcsum >= tlow) {
		#ifdef TEMPERATURE_OVER
		if (adcsum > tlow)
		return TEMPERATURE_OVER;
		#endif
		return TEMPERATURE_TABLE_START;
	}

	// Table lookup using binary search
	while ((r - l) > 1) {
		//temperature_table_index_type m = (l + r) >> 1;
		temperature_table_entry_type mid = TEMPERATURE_TABLE_READ((l + r) >> 1);
		if (adcsum > mid) {
			r = (l + r) >> 1;
		} else {
			l = (l + r) >> 1;
		}
	}
	temperature_table_entry_type vl = TEMPERATURE_TABLE_READ(l);
	if (adcsum >= vl) {
		return l * TEMPERATURE_TABLE_STEP + TEMPERATURE_TABLE_START;
	}
	temperature_table_entry_type vr = TEMPERATURE_TABLE_READ(r);
	temperature_table_entry_type vd = vl - vr;
	int16_t res = TEMPERATURE_TABLE_START + r * TEMPERATURE_TABLE_STEP;
	if (vd) {
		// Linear interpolation
		res -= ((TEMPERATURE_TABLE_STEP * (int32_t)(adcsum - vr) + (vd >> 1)) / vd);
	}
	return res;
}
/* ------------------------------------------------------------------------- */
