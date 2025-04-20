//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Contributors:	Steve Sims (refactoring)
// Created:			25/08/2023
// Last Updated:	25/08/2023
//

#ifndef AGON_VDP_PROTOCOL_H
#define AGON_VDP_PROTOCOL_H

#include <memory>
#include <HardwareSerial.h>

#include "agon.h"								// Configuration file

#define VDPSerial Serial2

void setVDPProtocolDuplex(bool duplex) {
	VDPSerial.setHwFlowCtrlMode(duplex ? HW_FLOWCTRL_CTS_RTS : HW_FLOWCTRL_RTS, 64);
}

void setupVDPProtocol() {
	VDPSerial.end();
	VDPSerial.setRxBufferSize(UART_RX_SIZE);					// Can't be called when running
	VDPSerial.begin(UART_BR, SERIAL_8N1, UART_RX, UART_TX);
	VDPSerial.setPins(UART_NA, UART_NA, UART_CTS, UART_RTS);	// Must be called after begin
	setVDPProtocolDuplex(false);								// Start with half-duplex
	VDPSerial.setTimeout(COMMS_TIMEOUT);
}

#endif // AGON_VDP_PROTOCOL_H
