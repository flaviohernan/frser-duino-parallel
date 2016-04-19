/*
 * This file is part of the frser-avr project.
 *
 * Copyright (C) 2009,2015 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */




#include "main.h"
#include "flash.h"
#include "uart.h"

/*********************************
 #################################
 *
 *				MEGA
 *
 #################################
 *********************************/

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

static uint8_t flash_databus_read(void) {
	uint8_t rv = 0x00;
	uint8_t aux = 0x00;
	rv = (PINB & 0x0C)>>2;
	aux = (PINL & 0x3F)<<2;
	rv |= aux;

	// rv = (PIND & 0xFC);
	// rv |= (PINB & 0x03);
	return rv;
}

static void flash_databus_tristate(void) {
	// DDRB &= ~(_BV(0) | _BV(1));
	// DDRD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
	// PORTB &= ~(_BV(0) | _BV(1));
	// PORTD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));

	DDRB &= ~(_BV(2) | _BV(3));
	DDRL &= ~(_BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5));
	PORTB &= ~(_BV(2) | _BV(3));
	PORTL &= ~(_BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5));
}

static void flash_databus_output(unsigned char data) {
	// PORTB = ((PORTB & 0xFC) | (data & 0x03));
	// PORTD = ((PORTD & 0x03) | (data & 0xFC));
	// DDRB |= (_BV(0) | _BV(1));
	// DDRD |= (_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));

	PORTB &= ~(_BV(2) | _BV(3));
	PORTB |= (data & 0x03)<<2;
	PORTL &= ~(_BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5));
	PORTL |= (data & 0XFC)>>2;

	DDRB |= (_BV(2) | _BV(3));
	DDRL |= (_BV(0) | _BV(1) | _BV(2) | _BV(3) | _BV(4) | _BV(5));

}

static void flash_chip_enable(void) {
	// PORTC &= ~_BV(3);

	PORTL &= ~_BV(6);
}

void flash_init(void) {
	// PORTC |= _BV(2);
	// DDRC |= (_BV(2) | _BV(1) | _BV(0));

	DDRL |= (_BV(6) | _BV(7)); // ce we
	DDRG |= (_BV(0)); // OE
	PORTL |= (_BV(6) | _BV(7)); // high ce we
	PORTG |= (_BV(0)); // high OE

	// init addressbus

	PORTA = 0X00;
	PORTC = 0X00;
	PORTD &= ~(_BV(7));
	PORTG &= ~(_BV(1) | _BV(2));

	DDRA = 0XFF;
	DDRC = 0XFF;
	DDRD |= _BV(7);
	DDRG |= _BV(1) | _BV(2);

	// PORTB &= ~_BV(2);
	// DDRB |= (_BV(4) | _BV(2));
	// ADDR unit init done
	// PORTC |= (_BV(3) | _BV(4) | _BV(5));
	// DDRC |= (_BV(3) | _BV(4) | _BV(5));
	// control bus init done
	// hmm, i should probably tristate the data bus by default...
	flash_databus_tristate();
	// CE control is not absolutely necessary...
	flash_chip_enable();
}

// // 'push' 3 bits of addr
// static void push_addr_bits(unsigned char bits) {
// /*	if (bits&4) PORTB |= _BV(4);
// 	else PORTB &= ~(_BV(4));
// 	if (bits&1) PORTC |= _BV(1);
// 	else PORTC &= ~(_BV(1));
// 	if (bits&2) PORTC |= _BV(0);
// 	else PORTC &= ~(_BV(0));*/
// 	// SET THE BITS: (functionality near as-above)
// 	asm volatile(
// 	"sbi %0, 4\n\t"
// 	"sbi %1, 1\n\t"
// 	"sbi %1, 0\n\t"
// 	"sbrs %2, 2\n\t"
// 	"cbi %0, 4\n\t"
// 	"sbrs %2, 0\n\t"
// 	"cbi %1, 1\n\t"
// 	"sbrs %2, 1\n\t"
// 	"cbi %1, 0\n\t"
// 	::
// 	"I" (_SFR_IO_ADDR(PORTB)),
// 	"I" (_SFR_IO_ADDR(PORTC)),
// 	"r" (bits)
// 	);
// 	// double-toggle CP
// 	asm volatile(
// 	"sbi %0, 2\n\t"
// 	"sbi %0, 2\n\t"
// 	::
// 	"I" (_SFR_IO_ADDR(PINB))
// 	);
// }



static void flash_output_enable(void) {
	// PORTC &= ~_BV(4);

	PORTG &= ~_BV(0);
}

static void flash_output_disable(void) {
	// PORTC |= _BV(4);

	PORTG |= _BV(0);
}

static const unsigned char BitReverseTable256[] = 
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0, 
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8, 
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4, 
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC, 
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2, 
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6, 
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9, 
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3, 
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7, 
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

static const unsigned char BitReverseTable4[] = 
{
	0x00, 0x02,
	0x01, 0x03
};

static void flash_setaddr(uint32_t addr) {

	uint8_t aux =  0;
	PORTA = 0X00;
	PORTC = 0X00;
	PORTD &= ~(_BV(7));
	PORTG &= ~(_BV(2) | _BV(1));

	PORTA = (addr & 0XFF);
	aux = addr >> 8;
	PORTC = BitReverseTable256[ aux & 0XFF ];
	aux =  0;
	aux = addr >> 9;
	PORTD |= (aux & 0x80);
	aux =  0;
	aux = addr >> 17;
	PORTG |= BitReverseTable4[ aux & 0X03 ] << 1;	
}

static void flash_pulse_we(void) {
	PORTL &= ~(_BV(7));
	asm volatile ("nop\n\t"
		"nop\n\t"
		"nop\n\t"
		::);
	PORTL |= _BV(7);
	
	// asm volatile(
	// "nop\n\t"
	// "sbi %0, 5\n\t"
	// "nop\n\t"
	// "sbi %0, 5\n\t"
	// ::
	// "I" (_SFR_IO_ADDR(PINC))
	// );
}

#endif


/*********************************
 #################################
 *
 *				UNO
 *
 #################################
 *********************************/


#if defined(__AVR_ATmega328P__)

static void flash_databus_tristate(void) {
	DDRB &= ~(_BV(0) | _BV(1));
	DDRD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
	PORTB &= ~(_BV(0) | _BV(1));
	PORTD &= ~(_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
}

static uint8_t flash_databus_read(void) {
	uint8_t rv;
	rv = (PIND & 0xFC);
	rv |= (PINB & 0x03);
	return rv;
}

static void flash_databus_output(unsigned char data) {
	PORTB = ((PORTB & 0xFC) | (data & 0x03));
	PORTD = ((PORTD & 0x03) | (data & 0xFC));
	DDRB |= (_BV(0) | _BV(1));
	DDRD |= (_BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7));
}

static void flash_chip_enable(void) {
	PORTC &= ~_BV(3);
}

void flash_init(void) {
	PORTC |= _BV(2);
	DDRC |= (_BV(2) | _BV(1) | _BV(0));
	PORTB &= ~_BV(2);
	DDRB |= (_BV(4) | _BV(2));
	// ADDR unit init done
	PORTC |= (_BV(3) | _BV(4) | _BV(5));
	DDRC |= (_BV(3) | _BV(4) | _BV(5));
	// control bus init done
	// hmm, i should probably tristate the data bus by default...
	flash_databus_tristate();
	// CE control is not absolutely necessary...
	flash_chip_enable();
}

// 'push' 3 bits of addr
static void push_addr_bits(unsigned char bits) {
/*	if (bits&4) PORTB |= _BV(4);
	else PORTB &= ~(_BV(4));
	if (bits&1) PORTC |= _BV(1);
	else PORTC &= ~(_BV(1));
	if (bits&2) PORTC |= _BV(0);
	else PORTC &= ~(_BV(0));*/
	// SET THE BITS: (functionality near as-above)
	asm volatile(
	"sbi %0, 4\n\t"
	"sbi %1, 1\n\t"
	"sbi %1, 0\n\t"
	"sbrs %2, 2\n\t"
	"cbi %0, 4\n\t"
	"sbrs %2, 0\n\t"
	"cbi %1, 1\n\t"
	"sbrs %2, 1\n\t"
	"cbi %1, 0\n\t"
	::
	"I" (_SFR_IO_ADDR(PORTB)),
	"I" (_SFR_IO_ADDR(PORTC)),
	"r" (bits)
	);
	// double-toggle CP
	asm volatile(
	"sbi %0, 2\n\t"
	"sbi %0, 2\n\t"
	::
	"I" (_SFR_IO_ADDR(PINB))
	);
}



static void flash_output_enable(void) {
	PORTC &= ~_BV(4);
}

static void flash_output_disable(void) {
	PORTC |= _BV(4);
}

static void flash_setaddr(uint32_t addr) {
	uint8_t i,n,d;
	// Currently uses 18-bit addresses
	for (i=6;i>0;i--) { // as i isn't really used here, this way generates faster & smaller code
		asm volatile(
		"mov %0, %C1\n\t"
		"lsl %0 \n\t"
		"bst %B1, 7\n\t"
		"bld %0, 0\n\t"
		: "=r" (n)
		: "r" (addr)
		);
//		push_addr_bits((addr>>15)&0x07);
		push_addr_bits(n); // ABOVE REPLACED BY ASM
		d = 3;
		asm volatile(
		"lsl %A0\n\t"
		"rol %B0\n\t"
		"rol %C0\n\t"
		"dec %1\n\t"
		"brne .-10\n\t"
		: "+r" (addr), "+r" (d)
		:
		);
		// addr = (addr<<3); // Done as 24-bit op in above asm + d=3
	}
}

static void flash_pulse_we(void) {
	asm volatile(
	"nop\n\t"
	"sbi %0, 5\n\t"
	"nop\n\t"
	"sbi %0, 5\n\t"
	::
	"I" (_SFR_IO_ADDR(PINC))
	);
}

#endif


/*********************************
 #################################
 *
 *         comom functions
 *
 #################################
 *********************************/



static void flash_read_init(void) {
	flash_databus_tristate();
	flash_output_enable();
}


// assume chip enabled & output enabled & databus tristate
static uint8_t flash_readcycle(uint32_t addr) {
	flash_setaddr(addr);
	asm volatile(
	"nop\n\t"
	"nop\n\t"
	:: ); // 250 ns @ 8 mhz // assembler inspection shows that these shouldn't be necessary
	return flash_databus_read();
}

// assume only CE, and perform single cycle
uint8_t flash_read(uint32_t addr) {
	uint8_t data;
	flash_read_init();
	flash_setaddr(addr);
	data = flash_databus_read();
	flash_output_disable();
	return data;
}

// assume only CE, perform single cycle
void flash_write(uint32_t addr, uint8_t data) {
	flash_output_disable();
	flash_databus_output(data);
	flash_setaddr(addr);
	flash_pulse_we();
}

void flash_readn(uint32_t addr, uint32_t len) {
	flash_read_init();
	do {
		SEND(flash_readcycle(addr++));
	} while(--len);
	// safety features
	flash_output_disable();
}


void flash_select_protocol(uint8_t allowed_protocols) {
	(void)allowed_protocols;
	flash_init();
}
