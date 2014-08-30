//Packet mater.  A super tiny file that helps with checksums and ethernet CRCs.
//CRC Was taken from linked page.

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

#ifndef _PACKETMAKER_H
#define _PACKETMAKER_H

#include <stdint.h>

struct EthernetPacket
{
	uint8_t preamble[8];
	uint8_t macto[6];
	uint8_t macfrom[6];
	uint16_t protocol;
	uint16_t header;

	//IP Header
	uint16_t len;
	uint16_t id;
	uint16_t flags;
	uint8_t ttl;
	uint8_t proto;
	uint16_t ipcsum;
	uint32_t addyfrom;
	uint32_t addyto;

	//UDP Header
	uint16_t srcport;
	uint16_t dstport;
	uint16_t length;
	uint16_t udpcsum;

	uint8_t payload[0];
};


//Fix all checksums for UDP packets and add etherlink CRC.
//You _MUST_ prepend your preamble before you get here.
//It should start off with 
//0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0xd5
// You MUST pass this in.
//
//plen is the "minimum" packet length.  if udplenoverride exceeds this, it will accomidate.
uint16_t Ethernetize( unsigned char * packet, int plen, int udplenoverride );

//From: http://www.hackersdelight.org/hdcodetxt/crc.c.txt
uint32_t crc32b(uint32_t crc, unsigned char *message, int len);


uint16_t internet_checksum( const unsigned char * start, uint16_t len );

#endif

