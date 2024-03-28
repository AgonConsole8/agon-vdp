90 REM PRINT "Before: ": VDU &89,&8B: PRINT "."
95 REM VDU 23,0,&C3: REM switch buffer
100 REM VDU 23,&89,&70,&80,&60,&10,&EA,&15,&15,&11: REM BDPP_PACKET_START/END_MARKER
110 REM VDU 23,&8B,&63,&94,&E2,&81,&66,&18,&20,&18: REM BDPP_PACKET_ESCAPE
130 REM PRINT "After: ": VDU &89,&8B: PRINT "."
190 OSCLI "BDPP"
200 DIM code% 300
201 DIM fcn% 1: PRINT "fcn%: ";~fcn%
202 DIM index% 1: PRINT "index%: ";~index%
203 DIM flags% 1: PRINT "flags%: ";~flags%
204 DIM size% 4: PRINT "size%: ";~size%
205 DIM count% 4: PRINT "count%: ";~count%
206 DIM data% 4: PRINT "data%: ";~data%
210 PROC_assemble_bdpp
220 ?fcn%=0: rc%=USR(bdppSig1%): PRINT "is_allowed -> ";rc%
230 ?fcn%=1: rc%=USR(bdppSig1%): PRINT "is_enabled -> ";rc%
250 PRINT "<<BDPP mode: This is in a packet!>>"
251 PRINT "Printing a 'Q' as one byte:";
252 ?fcn%=&0D: ?data%=&51: CALL bdppSig4%
253 PRINT ""
254 PRINT "This needs ESC characters:";
255 PRINT CHR$(&89);CHR$(&8B);"!"
300 DIM app_pkt% 1: PRINT "app_pkt%: ";~app_pkt%
310 ?app_pkt%=0: ?fcn%=5: ?index%=5: !data%=app_pkt%: !size%=1
311 PRINT "fcn=";?fcn%
312 PRINT "index=";?index%
313 PRINT "data=";~!data%
314 PRINT "size=";~!size%
315 REM hdr%=&0BC5B8
316 REM FOR pi%=0 TO 15
317 REM FOR di%=0 TO 11
318 REM PRINT " ";~?hdr%;: hdr%=hdr%+1
319 REM NEXT di%: PRINT "": NEXT pi%
320 rc%=USR(bdppSig6%): PRINT "rx pkt setup -> ";rc%
322 ?fcn%=&F: CALL bdppSig3%
323 REM hdr%=&0BC5B8
324 REM FOR pi%=0 TO 15
325 REM FOR di%=0 TO 11
326 REM PRINT " ";~?hdr%;: hdr%=hdr%+1
327 REM NEXT di%: PRINT "": NEXT pi%
330 VDU 23,0,&A2,1,5,&61
335 ?fcn%=&F: CALL bdppSig3%
340 ?index%=5: ?fcn%=7: rc%=USR(bdppSig2%)
350 IF rc%=0 GOTO 340
360 PRINT "echo returned -> ";~?app_pkt%
365 ?fcn%=&F: CALL bdppSig3%
370 PRINT "need to release pkt"
375 ?fcn%=&F: CALL bdppSig3%
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
