10 REM This program writes colored text onto the screen,
20 REM reads it using BDPP, and writes it back onto the screen.
30 REM It illustrates the use of BDPP, but does not use buffers
40 REM or packets in the most efficient manner.
100 MODE 3
140 DATA "*----------------------------------------------------------------------*"
150 DATA "| GRABBUFFER.BAS - Sample app shows using BDPP to grab VDP buffer data |"
160 DATA "| This program does the following things:                              |"
170 DATA "| (1) Write text on the screen in various colors                       |"
180 DATA "| (2) Capture a portion of the screen into a buffer on the ESP32       |"
190 DATA "| (3) Setup BDPP RX packets on the EZ80                                |"
200 DATA "| (4) Tell the ESP32 to transmit the buffer contents to the EZ80       |"
210 DATA "| (5) Draw the captured image upside-down at the bottom of the screen  |"
215 DATA "*----------------------------------------------------------------------*"
220 DATA ""
300 READ T$: IF T$="" GOTO 350
310 L%=LEN(T$)
320 FOR I%=1 TO L%
330 COLOUR RND(63): PRINT MID$(T$,I%,1);
340 NEXT I%: PRINT "": GOTO 300
350 VDU 23,0,&C0,0: REM Use normal (not logical) coordinates
360 OSCLI "BDPP": REM Turn on BDPP

390 REM Assemble the interfaces to the BDPP API
400 DIM code% 200
401 DIM fcn% 1
402 DIM index% 1
403 DIM flags% 1
404 DIM size% 4
405 DIM count% 4
406 DIM data% 4
407 packet_size%=256
410 PROC_assemble_bdpp

510 REM Create a local RX packet buffer for a scan line (576 bytes)
520 DIM buffer% 576

590 REM Capture upper portion of screen (the colored text)
600 capWidth%=72*8: capHeight%=9*8: bufferId%=64001: lineBufferId%=64002
610 MOVE 0,0: MOVE capWidth%-1,capHeight%-1
620 VDU 23,27,1,bufferId%,0,0;
630 ?fcn%=&F: CALL bdppSig3%

700 offsetHi%=0: offsetLo%=0
710 FOR i%=0 TO capHeight%-1
715 PRINT "(";i%;")";: ?fcn%=&F: CALL bdppSig3%

731 REM Request 3 sections of the captured pixels
732 ?fcn%=5: ?index%=0: !data%=buffer%: !size%=256
733 rc%=USR(bdppSig6%)
734 ?fcn%=5: ?index%=1: !data%=buffer%+256: !size%=256
735 rc%=USR(bdppSig6%)
736 ?fcn%=5: ?index%=2: !data%=buffer%+512: !size%=64
737 rc%=USR(bdppSig6%)

740 VDU 23,0,&A0,bufferId%;&1B,0,offsetLo%;offsetHi%;256;
742 VDU 23,0,&A0,bufferId%;&1B,1,offsetLo%+256;offsetHi%;256;
744 VDU 23,0,&A0,bufferId%;&1B,2,offsetLo%+512;offsetHi%;64;
750 ?fcn%=&F: CALL bdppSig3%

760 offsetLo%=offsetLo%+capWidth%

770 REM Wait for the response packets with pixel data
772 ?index%=0: ?fcn%=7
774 rc%=USR(bdppSig2%): IF rc%=0 GOTO 774
776 ?index%=1: ?fcn%=7
778 rc%=USR(bdppSig2%): IF rc%=0 GOTO 778
780 ?index%=2: ?fcn%=7
782 rc%=USR(bdppSig2%): IF rc%=0 GOTO 782

793 REM PRINT "<";?buffer%;">": END

795 REM Write the pixels to a single line buffer
800 VDU 23,0,&A0,lineBufferId%;0,capWidth%;

802 address%=buffer%
804 FOR B%=1 TO capWidth%
806 VDU ?address%: address%=address%+1
808 NEXT B%
810 ?fcn%=&F: CALL bdppSig3%

871 REM address%=buffer%: PRINT i%;": ";
872 REM FOR B%=0 TO 15
873 REM PRINT STR$(?address%);" ";: address%=address%+1
874 REM NEXT B%
875 REM PRINT ""
876 REM ?fcn%=&F: CALL bdppSig3%

900 VDU 23,27,&20,lineBufferId%;: REM Select bitmap (using a buffer ID)
910 VDU 23,27,&21,capWidth%;1;1: REM Create bitmap from buffer
920 VDU 25,&ED,0;(230-i%);: REM Show the bitmap on the screen line
930 lineBufferId%=lineBufferId%+1
960 NEXT i%

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
50244 ENDPROC
