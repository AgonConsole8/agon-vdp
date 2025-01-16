#ifndef HEXLOAD_H
#define HEXLOAD_H

#include <stdbool.h>
#ifndef USERSPACE
#include "CRC16.h"
#include "CRC32.h"
#endif /* !USERSPACE */

extern void printFmt(const char *format, ...);
extern HardwareSerial DBGSerial;

#ifndef USERSPACE
CRC16 linecrc16(0x8005, 0x0, 0x0, false, false);
CRC32 crc32,crc32tmp;
#endif /* !USERSPACE */
bool aborted;

#define DEF_LOAD_ADDRESS			0x040000
#define DEF_U_BYTE  				((DEF_LOAD_ADDRESS >> 16) & 0xFF)
#define OVERRUNTIMEOUT				5
#define IHEX_RECORD_DATA			0
#define IHEX_RECORD_EOF				1
#define IHEX_RECORD_SEGMENT			2 //Extended Segment Address record
#define IHEX_RECORD_LINEAR			4 //Extended Linear address record,
#define IHEX_RECORD_EXTENDEDMODE	0xFF

void VDUStreamProcessor::sendKeycodeByte(uint8_t b, bool waitforack) {
	uint8_t packet[] = {b,0};
	send_packet(PACKET_KEYCODE, sizeof packet, packet);                    
	if(waitforack) readByte_b();
}

uint8_t serialRx_t(void) {
	auto kb = getKeyboard();
	fabgl::VirtualKeyItem item;
	uint32_t start = millis();

	if (kb->getNextVirtualKey(&item, 0)) {
		if(item.down) {
			if(item.ASCII == 0x1B) {
				aborted = true;
				return 0;
			}
		}
	}

	while(DBGSerial.available() == 0) {
		if((millis() - start) > OVERRUNTIMEOUT) return 0;
	}
	return DBGSerial.read();
}

void consumeHexMarker(void) {
	uint8_t data = 0;
	while(data != ':') {
		data = serialRx_t();
		if(aborted) return;
	}
}

// Receive a single iHex Nibble from the external Debug serial interface
uint8_t getIHexNibble(bool addcrc) {
#ifndef USERSPACE
	uint8_t nibble, input;
	input = toupper(serialRx_t());
	if(addcrc) linecrc16.add(input);
	if((input >= '0') && input <='9') nibble = input - '0';
	else nibble = input - 'A' + 10;
	// illegal characters will be dealt with by checksum later
	return nibble;
#endif /* !USERSPACE */
}

// Receive a byte from the external Debug serial interface as two iHex nibbles
uint8_t getIHexByte(bool addcrc) {
	uint8_t value;
	value = getIHexNibble(addcrc) << 4;
	value |= getIHexNibble(addcrc);
	return value;  
}

uint16_t getIHexUINT16(bool addcrc) {
	uint16_t value;
	value =  ((uint16_t)getIHexByte(addcrc)) << 8;
	value |= getIHexByte(addcrc);
	return value;
}

uint32_t getIHexUINT32(bool addcrc) {
	uint32_t value;
	value =  ((uint32_t)getIHexUINT16(addcrc)) << 16;
	value |= getIHexUINT16(addcrc);
	return value;
}

void echo_checksum(uint8_t linechecksum, uint8_t ez80checksum, bool retransmit) {
	if(retransmit) printFmt("R");
	if(ez80checksum) printFmt("*");
	if(linechecksum) {printFmt("X"); return;}
	printFmt(".");
}

void serialTx(uint16_t crc) {
	DBGSerial.write((uint8_t)(crc & 0xFF));
	DBGSerial.write((uint8_t)(((crc >> 8) & 0xFF)));
}

void serialTx(uint32_t crc) {
	serialTx((uint16_t)(crc & 0xFFFF));
	serialTx((uint16_t)((crc >> 16) & 0xFFFF));
}

void VDUStreamProcessor::vdu_sys_hexload(void) {
#ifdef USERSPACE
	// no hexload for emulators :)
	return;
#else /* !USERSPACE */
	uint32_t 	segment_address;
	uint32_t 	crc32target;
	uint8_t 	u,h,l,tmp;
	uint8_t 	bytecount;
	uint8_t 	recordtype,subtype;
	uint8_t 	data;
	uint8_t 	linechecksum,ez80checksum;
	bool		extendedformat;
	bool 		done,printdefaultaddress,segmentmode,no_startrecord;
	bool 		retransmit;
	bool 		rom_area;
	uint16_t 	errorcount;

	printFmt("Receiving Intel HEX records - VDP:%d 8N1\r\n\r\n", SERIALBAUDRATE);

	aborted = false;
	u = DEF_U_BYTE;
	errorcount = 0;
	done = false;
	printdefaultaddress = true;
	segmentmode = false;
	no_startrecord = false;
	rom_area = false;

	crc32.restart();
	crc32tmp.restart();
	crc32target = 0;
	extendedformat = false;

	retransmit = false;
	while(!done) {
		linecrc16.restart();
		consumeHexMarker();
		linecrc16.add(':');
		if(aborted) {
			printFmt("\r\nAborted\r\n");
			sendKeycodeByte(0, true); // Release caller
			return;
		}

		// Get frame header
		bytecount = getIHexByte(true);  // number of bytes in this record
		h = getIHexByte(true);      	// middle byte of address
		l = getIHexByte(true);      	// lower byte of address 
		recordtype = getIHexByte(true); // record type

		linechecksum = bytecount + h + l + recordtype;  // init control checksum
		if(segmentmode) {
			u = ((segment_address + (((uint32_t)h << 8) | l)) & 0xFF0000) >> 16;
			h = ((segment_address + (((uint32_t)h << 8) | l)) & 0xFF00) >> 8;
			l = (segment_address + (((uint32_t)h << 8) | l)) & 0xFF;
		}
		ez80checksum = 1 + u + h + l + bytecount; 		// to be transmitted as a potential packet to the ez80

		switch(recordtype) {
			case IHEX_RECORD_DATA:
				if(printdefaultaddress) {
					printFmt("\r\nAddress 0x%02x0000 (default)\r\n", DEF_U_BYTE);
					printdefaultaddress = false;
					no_startrecord = true;
				}
				sendKeycodeByte(1, true);			// ez80 data-package start indicator
				sendKeycodeByte(u, true);      		// transmit full address in each package  
				sendKeycodeByte(h, true);
				sendKeycodeByte(l, true);
				sendKeycodeByte(bytecount, true);	// number of bytes to send in this package
				while(bytecount--) {
					data = getIHexByte(true);
					crc32tmp.add(data);
					sendKeycodeByte(data, false);
					linechecksum += data;			// update linechecksum
					ez80checksum += data;			// update checksum from bytes sent to the ez80
				}
				ez80checksum += readByte_b();		// get feedback from ez80 - a 2s complement to the sum of all received bytes, total 0 if no errorcount      
				linechecksum += getIHexByte(true);		// finalize checksum with actual checksum byte in record, total 0 if no errorcount
				if(linechecksum || ez80checksum) errorcount++;
				if(u >= DEF_U_BYTE) echo_checksum(linechecksum,ez80checksum,retransmit);
				else printFmt("!");
				break;
			case IHEX_RECORD_SEGMENT:
				printdefaultaddress = false;
				segmentmode = true;
				tmp = getIHexByte(true);              // segment 16-bit base address MSB
				linechecksum += tmp;
				segment_address = tmp << 8;
				tmp = getIHexByte(true);              // segment 16-bit base address LSB
				linechecksum += tmp;
				segment_address |= tmp;
				segment_address = segment_address << 4; // resulting segment base address in 20-bit space
				tmp = getIHexByte(true);
				linechecksum += tmp;		// finalize checksum with actual checksum byte in record, total 0 if no errorcount
				if(linechecksum) errorcount++;
				echo_checksum(linechecksum,0,retransmit);		// only echo local checksum errorcount, no ez80<=>ESP packets in this case

				if(no_startrecord) {
					printFmt("\r\nSegment address 0x%06X", segment_address);
					segment_address += DEF_LOAD_ADDRESS;
					printFmt(" - effective 0x%06X\r\n", segment_address);
				}
				else printFmt("\r\nAddress 0x%06X\r\n", segment_address);
						if(segment_address < DEF_LOAD_ADDRESS) {
					printFmt("ERROR: Address in ROM area\r\n", segment_address);
					rom_area = true;
				}
				ez80checksum = 0;
				break;
			case IHEX_RECORD_EOF:
				tmp = getIHexByte(true);
				sendKeycodeByte(0, true);       	// end transmission
				done = true;
				ez80checksum = 0;
				break;
			case IHEX_RECORD_LINEAR:  // only update U byte for next transmission to the ez80
				printdefaultaddress = false;
				segmentmode = false;
				tmp = getIHexByte(true);
				linechecksum += tmp;		// ignore top byte of 32bit address, only using 24bit
				u = getIHexByte(true);
				linechecksum += u;
				tmp = getIHexByte(true);
				linechecksum += tmp;		// finalize checksum with actual checksum byte in record, total 0 if no errorcount
				if(linechecksum) errorcount++;
				echo_checksum(linechecksum,0,retransmit);	// only echo local checksum errorcount, no ez80<=>ESP packets in this case
				if(u >= DEF_U_BYTE) printFmt("\r\nAddress 0x%02X0000\r\n", u);
				else {
					printFmt("\r\nERROR: Address 0x%02X0000 in ROM area\r\n", u);
					rom_area = true;
				}
				ez80checksum = 0;
				break;
			case IHEX_RECORD_EXTENDEDMODE:
				ez80checksum = 0;
				getIHexByte(true);
				subtype = getIHexByte(true);
				switch(subtype) {
					case 0:
						extendedformat = true;
						crc32target = getIHexUINT32(true);
						getIHexByte(true);
						printFmt("Extended mode\r\n");
						break;
					default:
						break;
				}
				break;
			default: // ignore other (non I32Hex) records
				break;
		}

		if(extendedformat) {
			uint16_t receivecrc16 = getIHexUINT16(false);

			if(ez80checksum) { // even if CRCs match, hexload client at ez80 might trigger a single-bit checksum error
				retransmit = true;
				crc32tmp = crc32;
				linecrc16.add(1); // trigger retransmit
			}
			else {
				if(receivecrc16 == linecrc16.calc()) {
					retransmit = false;
					crc32 = crc32tmp;
				}
				else {
					retransmit = true;
					crc32tmp = crc32;
				}
			}
			serialTx(linecrc16.calc());
		}
	}

	if(extendedformat) {
		serialTx(crc32.calc());
		if(crc32.calc() == crc32target) printFmt("\r\n\r\nCRC32 OK (0x%08X)\r\n", crc32.calc());
		else printFmt("\r\n\r\nCRC32 ERROR");
	}
	else {
		printFmt("\r\n\r\nOK\r\n");
		if(errorcount) {
			printFmt("\r\n%d error(s)\r\n",errorcount);
		}
	}
	if(rom_area) printFmt("\r\nHEX data overlapping ROM area, transfer unsuccessful\r\nERROR\r\n");
	printFmt("VDP done\r\n");   
#endif /* !USERSPACE */
}

#endif // HEXLOAD_H
