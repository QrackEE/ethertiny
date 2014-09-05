/*

	WARNING: Connecting an AVR to ethernet without magnetics is VERY DANGEROUS.
	It may cause high voltages to go through the AVR and destroy it in DANGEROUS
	that may harm personal safety.  Do only at your own risk.

*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "net_compat.h"
//#include "sendpack.h"
#include "packetmater.h"
#include "hlprocess.h"
#include "iparpetc.h"

void delay_ms(uint32_t time) {
  uint32_t i;
  for (i = 0; i < time; i++) {
    _delay_ms(1);
  }
}

#define NOOP asm volatile("nop" ::)

static void setup_clock( void )
{
	/*Examine Page 33*/

	CLKPR = 0x80;	/*Setup CLKPCE to be receptive*/
	CLKPR = 0x00;	/*No scalar*/
}

//#define SMARTPWR


void SendTestASM( const unsigned char * c, uint8_t len );
int MaybeHaveDataASM( unsigned char * c, uint8_t lenX2 ); //returns the number of pairs.

uint8_t MyMAC[6] = { 0x00, 0x55, 0x00, 0x55, 0x00, 0x55 };
uint8_t MyIP[4] = { 192, 168, 0, 153 };
uint8_t MyMask[4] = { 255, 255, 255, 0 };

/////////////////////////////////Sending for WS2812B///////////////////////////

#define WSPORT PORTB
#define WSPIN  _BV(3)
#define WSDDR  DDRB

//SEND_WS at 20MHz on WS2812B

#define SEND_WS( var ) \
				mask = 0x80; \
				v = var; \
				while( mask ) \
				{ \
					if( mask & v )  \
					{ \
						WSPORT |= WSPIN;  mask>>=1;   \
						NOOP; NOOP; NOOP; NOOP; \
						NOOP; NOOP; NOOP; NOOP;NOOP;NOOP;\
						WSPORT &= ~WSPIN; NOOP; NOOP; NOOP; NOOP;\
					} \
					else \
					{ \
						WSPORT |= WSPIN; NOOP; NOOP;\
						WSPORT &= ~WSPIN; \
						mask>>=1; \
						NOOP; NOOP; NOOP; NOOP;NOOP; NOOP; NOOP;\
					} \
					\
				}



void HandleUDP( uint16_t len )
{
	uint8_t mask;
	uint8_t v;
	/*uint16_t epcs = */et_pop16(); //Checksum
	len -= 8; //remove header.

	if( localport == 13313 )
	{
		uint8_t i;

		for( i = 0; i < len; i++ )
		{
			SEND_WS( et_pop8() );
		}

		WSPORT &= ~WSPIN;
	}

}

void SendNLP();
volatile uint8_t timercycle;
//Called ~1x every 4ms.

ISR(TIM1_COMPA_vect)
{
	timercycle++;
	if( (timercycle & 3) == 0 )
	{
		SendNLP();
	}
}


ISR(PCINT0_vect )
{
	MaybeHaveAPacket();
	GIFR |= _BV(PCIF); //The flag has likely been set from within.
}


int main( )
{
	uint8_t frame;
	static uint8_t i;
	static uint16_t adcT = 0;  //ADC Temp
	static uint16_t adcP = 0;  //ADC Pin
	static uint8_t last_timercycle = 0;
	cli();

	setup_clock();

	//1st let's see how fast we can clock the pin.
	et_init( MyMAC );

	i = 0;


	//Assumed setup:
	//
	// PB2: Disconnect this (I don't know if you can use it at all)
	// PB1: TX (From this device, out) Put a 47nF capcaitor in series.

	//Enable ADC.  (For reading in pin ADC2)
	PORTB &= ~_BV(4);
	PORTB |= _BV(4);
	ADCSRA = _BV(ADEN) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADSC) | _BV(ADATE);

	//Enable port B for the WS2812B.
	WSDDR |= WSPIN;
	WSPORT &= ~WSPIN;

	//The burden of configuring the tick timer is on you, the user.
	//#188 Creates a clock with period ~4ms. ~3.2ms at 31MHz, ~4.8ms at 20 MHz.
	//Making it lower seems to help.
	TCCR1 = _BV(CTC1) | _BV(CS13) | _BV(CS12); //HS Clock/2048
	OCR1C = 255; 
	OCR1A = 1;
	TIMSK = _BV(OCIE1A);

	//Enable Pin Change Interrupt.
	//(For if we have some RX data)
	PCMSK |= _BV(PCINT0);
	GIMSK |= _BV(PCIE);

	sei();

	OSCCAL = OSCHIGH;
	PORTB |= _BV(0);

	while(1)
	{
		if( last_timercycle != timercycle )
		{
			last_timercycle = timercycle;
			//i++;
			i = timercycle & 0x0f;

			if( i == 0 )
			{
				ADMUX = 2;
			}

			if( i == 1 )
			{
				adcP = ADC;
				ADMUX = _BV(REFS1) | 0x0f;
			}

			if( i == 2 )
			{
				adcT = ADC;
			}

			if( i == 14 )
			{

				frame++;
				//How to send a UDP Packet.
				cli();
				OSCCAL = OSC20;
				et_stopop();
				et_startsend( 0 );
				memset( macfrom, 0xff, 6 );
				send_etherlink_header( 0x0800 );
				send_ip_header( 0, (unsigned char*)"\xff\xff\xff\xff", 17 ); //UDP Packet to 255.255.255.255
				et_push16( 13313  ); //to port
				et_push16( 13312 ); //from port
				et_push16( 0 ); //length for later
				et_push16( 0 ); //csum for later
				et_pushpgmstr( PSTR( "TPIN" ) ); //csum for later
				et_push16( adcT ); 
				et_push16( adcP ); 
				et_push16( icmp_out ); 
				et_push16( hict ); 
				et_push16( lowct );
				util_finish_udp_packet();
				OSCCAL = OSCHIGH;
				sei();
				i = 0;

			}

		}
	}



	return 0;
} 
/*
    Copyright (C) 2014 <>< Charles Lohr


    Permission is hereby granted, free of charge, to any person obtaining a
	copy of this software and associated documentation files (the "Software"),
	to deal in the Software without restriction, including without limitation
	the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the
	Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
	in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
	OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
	MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
	TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
	SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

