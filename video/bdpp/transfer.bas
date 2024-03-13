10 REM Sample program of BDPP data transfer
20 REM Read packets of data from ESP32 SRAM0 memory bank
30 REM Illustrates have multiple outstanding requests,
40 REM and awaiting responses to those requests.
100 OSCLI "bdpp"
200 DIM code% 300
201 DIM fcn% 1: PRINT "fcn%: ";~fcn%
202 DIM index% 1: PRINT "index%: ";~index%
203 DIM flags% 1: PRINT "flags%: ";~flags%
204 DIM size% 4: PRINT "size%: ";~size%
205 DIM count% 4: PRINT "count%: ";~count%
206 DIM data% 4: PRINT "data%: ";~data%
207 packet_size%=10
210 PROC_assemble_bdpp
215 REM Setup a large (4KB) RX buffer, and divide it
220 REM into 16 RX packets of packet_size% bytes each.
230 DIM buffer% 4096: REM Max packet_size% of 256 bytes
235 PRINT "Setting up 16 RX packets of ";packet_size%;" bytes each"
240 FOR pi%=0 TO 15
250 ?fcn%=5: ?index%=pi%: !data%=(buffer%+pi%*packet_size%): !size%=packet_size%
260 rc%=USR(bdppSig6%)
265 PRINT "RX pkt #";pi%;" at ";~!data%;" setup -> ";rc%
270 ?fcn%=&F: CALL bdppSig3%
280 NEXT pi%
282 PRINT " done."
284 PRINT "Sending 16 requests for ";packet_size%;" bytes each...";
286 ?fcn%=&F: CALL bdppSig3%
288 ts=TIME
300 FOR pi%=0 TO 15
310 VDU 23,0,&A2,3,pi%,&E000+pi%*packet_size%;&3FFA;packet_size%;
320 NEXT pi%
322 PRINT " done."
324 ?fcn%=&F: CALL bdppSig3%
330 FOR pi%=0 TO 15
340 PRINT "Waiting for RX pkt #";pi%;"...";
350 ?fcn%=&F: CALL bdppSig3%
360 ?index%=pi%: ?fcn%=7
365 rc%=USR(bdppSig2%)
370 IF rc%=0 GOTO 365
380 PRINT " arrived!"
385 ?fcn%=&F: CALL bdppSig3%
390 NEXT pi%
400 tt=TIME-ts
410 PRINT "Transfer time = ";tt/100;" seconds"
420 ?fcn%=&F: CALL bdppSig3%
430 FOR pi%=0 TO 15
450 ?fcn%=&17: ?index%=pi%: rc%=USR(bdppSig2%)
460 PRINT "Release RX pkt #";pi%;" -> ";rc%
470 NEXT pi%
475 ?fcn%=&F: CALL bdppSig3%
999 END

49985 REM IX: Data address
49986 REM IY: Size of buffer or Count of bytes
49987 REM  D: Data byte
49988 REM  C: Packet Flags
49989 REM  B: Packet Index(es)
49990 REM  A: BDPP function code
50000 DEF PROC_assemble_bdpp
50001 REM -- bdppSig1% --
50002 REM BDPP uses several styles of function calls:
50004 REM BOOL bdpp_is_allowed();
50006 REM BOOL bdpp_is_enabled();
50010 REM BOOL bdpp_disable();
50012 P%=code%: bdppSig1%=P%
50014 [OPT 2
50015 LD HL,fcn%: LD A,(HL)
50016 RST.LIL &20
50017 LD L,A
50018 LD H,0
50019 XOR A
50020 EXX
50021 XOR A
50022 LD L,0
50023 LD H,0
50024 LD C,A
50025 RET
50026 ]
50039 REM -- bdppSig2% --
50040 REM BOOL bdpp_enable(BYTE stream);
50042 REM BOOL bdpp_is_tx_app_packet_done(BYTE index);
50044 REM BOOL bdpp_is_rx_app_packet_done(BYTE index);
50046 REM BYTE bdpp_get_rx_app_packet_flags(BYTE index);
50048 REM BOOL bdpp_stop_using_app_packet(BYTE index);
50050 bdppSig2%=P%
50051 [OPT 2
50052 LD HL,index%: LD B,(HL)
50053 LD HL,fcn%: LD A,(HL)
50054 RST.LIL &20
50055 LD L,A
50056 LD H,0
50057 XOR A
50058 EXX
50059 XOR A
50060 LD L,0
50061 LD H,0
50062 LD C,A
50063 RET
50064 ]
50079 REM -- bdppSig3% --
50080 REM void bdpp_flush_drv_tx_packet();
50082 bdppSig3%=P%
50084 [OPT 2
50085 LD HL,fcn%: LD A,(HL)
50086 RST.LIL &20
50088 RET
50090 ]
50099 REM -- bdppSig4% --
50100 REM void bdpp_write_byte_to_drv_tx_packet(BYTE data);
50101 REM void bdpp_write_drv_tx_byte_with_usage(BYTE data);
50102 bdppSig4%=P%
50104 [OPT 2
50109 LD HL,fcn%: LD A,(HL)
50110 LD HL,data%: LD D,(HL)
50112 RST.LIL &20
50114 RET
50116 ]
50119 REM -- bdppSig5% --
50120 REM void bdpp_write_bytes_to_drv_tx_packet(const BYTE* data, WORD count);
50122 REM void bdpp_write_drv_tx_bytes_with_usage(const BYTE* data, WORD count);
50124 bdppSig5%=P%
50126 [OPT 2
50128 LD HL,count%: DEFB &ED: DEFB &31
50130 LD HL,fcn%: LD A,(HL)
50132 LD HL,data%: DEFB &ED: DEFB &37
50138 RST.LIL &20
50140 RET
50142 ]
50159 REM -- bdppSig6% --
50160 REM BOOL bdpp_prepare_rx_app_packet(BYTE index, BYTE* data, WORD size);
50161 bdppSig6%=P%
50162 [OPT 2
50163 LD HL,size%: DEFB &ED: DEFB &31
50164 LD HL,index%: LD B,(HL)
50165 LD HL,fcn%: LD A,(HL)
50166 LD HL,data%: DEFB &ED: DEFB &37
50167 RST.LIL &20
50168 LD L,A
50169 LD H,0
50170 XOR A
50171 EXX
50172 XOR A
50173 LD L,0
50174 LD H,0
50175 LD C,A
50176 RET
50178 ]
50199 REM -- bdppSig7% --
50200 REM BOOL bdpp_queue_tx_app_packet(BYTE indexes, BYTE flags, const BYTE* data, WORD size);
50201 bdppSig7%=P%
50202 [OPT 2
50203 LD HL,size%: DEFB &ED: DEFB &31
50204 LD HL,flags%: LD D,(HL)
50205 LD HL,index%: LD B,(HL)
50206 LD HL,fcn%: LD A,(HL)
50207 LD HL,data%: DEFB &ED: DEFB &37
50208 RST.LIL &20
50209 LD L,A
50210 LD H,0
50211 XOR A
50212 EXX
50213 XOR A
50214 LD L,0
50215 LD H,0
50216 LD C,A
50217 RET
50218 ]
50219 REM -- bdppSig8% --
50230 REM WORD bdpp_get_rx_app_packet_size(BYTE index);
50231 bdppSig8%=P%
50232 [OPT 2
50234 LD HL,index%: LD B,(HL)
50236 LD HL,fcn%: LD A,(HL)
50238 RST.LIL &20
50240 RET
50242 ]

50244 PRINT "bdppSig1% ";~bdppSig1%
50246 PRINT "bdppSig2% ";~bdppSig2%
50248 PRINT "bdppSig3% ";~bdppSig3%
50250 PRINT "bdppSig4% ";~bdppSig4%
50252 PRINT "bdppSig5% ";~bdppSig5%
50254 PRINT "bdppSig6% ";~bdppSig6%
50256 PRINT "bdppSig7% ";~bdppSig7%
50258 PRINT "total code: ";P%-code%
50260 ENDPROC
