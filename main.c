/**
 * Ledikuutio Main...
 * TODO: 	Make sure that SRAM usage stays as low as possible,
 * 			avoid using local variables or declaring variables at runtime.
 *
 * TODO: 	Check if there's way to monitor the stack usage at runtime
 * 			to avoid stack overflow when pushing close the SRAM limit.
 *
 */

#include <avr/interrupt.h>
#include <avr/io.h>
#include "pinMacros.h"
#include "main.h"
#include "init.h"
#include "tlc5940.h"

volatile uint16_t c=0; //Global testing variable...

//Grayscale data array, lenght is 24 * number of devices * number of layers in a cube...
//TODO: currently fixed to 1 device, calculate from number of devices an fill with values
//TODO: remember to clear these arrays or initialize them to 0 before blank goes low.
uint8_t GSdata[24*TLC5940]={0x00};

uint8_t GSdata2[24*TLC5940]={0x00};

uint8_t *Midbuffer = GSdata2; //TODO: arrange the code more logically, this should be moved...

//TODO: backbuffer for double buffering...
uint8_t *BackBuffer = GSdata2;

//Pointer to the GS data buffer that holds the data to be sent to the TLC5940
uint8_t *FrontBuffer = GSdata;

//USART variables...
#define BUF_SIZE 64
#define RX_OK 0
#define RX_OVERFLOW 1
#define ESCAPE 0x7e

#define DEFAULT 0x00
#define STANDALONE 0x01
#define SERIAL_SLAVE 0x02

uint8_t state=0x00;

uint8_t rx_buf[BUF_SIZE];
volatile uint8_t rx_in_i = 0;
uint8_t rx_out_i = 0;

volatile uint8_t rx_state = RX_OK;

uint8_t i = 1;
uint8_t apu = 1; //we need this in order to determine if the non-skipped number is odd

int main() {

	cli();

	disableWDT();
	initPorts();
	initSPI();

	initTLC5940();
	initBLANKTimer();

	initUSART();
	sei();

	InitGScycle(); //TODO: Send first byte to the SPI bus...

	while(1){

//		if (isAfterFlip) {
//		}

		if(serial_available()){

			if(state==DEFAULT){
				//wait command from usart and process it

				processCommand();
			}
			else{
				stateMachine();
			}

		}

		if(state == STANDALONE){
			animSnake();
		}

		if(state == SERIAL_SLAVE){
			//put the current byte to the backbuffer unelss it's literal escape
		}

	}

	return 0;
}

void animSnake(){
	// Clear backbuffer once every frame...
		if (isAfterFlip) {
			clearArray(BackBuffer, 24*TLC5940);

			if (i < (25*TLC5940)) {

				if(i%3==0){ //Skip!
					i++;
					apu=1; //we need to reset the helper
				}


				if(apu==1){ //Odd
					BackBuffer[i-1]=0xff;
					BackBuffer[i]=0xf0;
				}else{//even
					BackBuffer[i-1]=0x0f;
					BackBuffer[i]=0xff;
				}
				apu++;

			}

			i++;

			if(i==24*TLC5940){ //Ending cell, reset EVERYTHING
				i=1;
				apu=1;
				//clearArray(BackBuffer, 24*TLC5940);
			}

			isAfterFlip = 0;
		}
}

void processCommand(){
	switch (serial_read()) {
		case ESCAPE:

				state = DEFAULT;
			break;

		case STANDALONE:

				state = STANDALONE;
			break;

		case SERIAL_SLAVE:

				state = SERIAL_SLAVE;
			break;

		default:
			//keep the current state
			break;
	}
}

void stateMachine(){
	switch (serial_read()) {
		case ESCAPE:
			if(serial_read()==0x00){
				//literal escape
				//state = 0x00
			}
			else{
				state = 0x00;
			}
			break;

		default:
			//do whatever in the current state
			break;
	}
}

void clearArray(volatile uint8_t *arr, uint8_t len) {

	for (uint8_t r = 0; r < len; r++) {
		arr[r] = 0x00;
	}

}

//Send byte via USART
void USART_Transmit(uint8_t data)
{
	/* Wait for empty transmit buffer*/
	while( !( UCSR0A & (1<<UDRE0)) );
	UDR0 = data;
	/* Put data into buffer, sends the data*/

}

//USART Received byte vector
//ISR(USART_RX_vect)
//{
//	TXpuskuri[i]=UDR0;
//	if(i>=8){
//		i=0;
//	}
//	UDR0 = TXpuskuri[i];
//	i++;
//	//SPI_MasterTransmit(TXpuskuri);
//}
////Generic SPI master transmission
////Return slave data.
//void SPI_Transfer(uint8_t cData)
//{
//	/* Start transmission */
//	PORTB |= (1<<PB1);
//	SPDR = cData; //Send byte
//	/* Wait for transmission complete */
//	while(!(SPSR & (1<<SPIF)));
//	PORTB &= ~(1<<PB1);
//}

////SPI Transfer for TLC5940
//void SPI_Transfer_TLC5940(uint8_t FrontBuffer[])
//{
//	GSdataCounter=0;
//	/* Start transmission */
//	PORTB &= ~(1<<PB1);
//
//	//notTransferring=0; //we're transferring data...
//	SPDR = FrontBuffer[0]; //Send first byte to initialize the ISR managed transfer
//
//	/* Wait for transmission complete */
//	//while(!(SPSR & (1<<SPIF)));
//	//PORTB &= ~(1<<PB2);
//}

//USART Received byte vector
ISR(USART_RX_vect)
{
	rx_buf[rx_in_i++] = UDR0;

	// Wrap to start
	if (rx_in_i == BUF_SIZE) rx_in_i = 0;
	
	if (rx_in_i == rx_out_i) {
		// Overflow condition
		rx_state = RX_OVERFLOW;
	}
}

//USART Transmit complete interrupt
ISR(USART_TX_vect)
{
	//TODO: Implement usart interrupt driven transmission
	//Get next byte from TX buffer and put it to USART transmit buffer
	//Manage buffer
}

/**
 * Returns the number of bytes available in receive buffer
 */
uint8_t serial_available(void) {
	uint8_t diff = rx_in_i - rx_out_i;
	return (rx_in_i < rx_out_i) ? diff + BUF_SIZE : diff;
}

/**
 * Empties receive buffer.
 */
void serial_empty(void) {
	rx_out_i = rx_in_i;
}

/**
 * Reads a byte from receive buffer. Does not check underrun
 * condition; the user is responsible to check serial_available()
 * before calling this.
 */
uint8_t serial_read(void) {
	uint8_t data = rx_buf[rx_out_i++];
	if (rx_out_i == BUF_SIZE) rx_out_i = 0;
	return data;
}

//If an interrupt happens and there isn't an interrupt handler, we go here!
ISR(BADISR_vect)
{
	pin_high(DEBUG_LED); //Give us an indication about an error condition...
	while(1){
		}
}
