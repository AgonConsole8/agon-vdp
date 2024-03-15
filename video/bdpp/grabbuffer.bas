10 REM
20 REM
30 REM
100 MODE 3
110 REM OSCLI "BDPP"
140 DATA "+----------------------------------------------------------------------+"
150 DATA "| GRABBUFFER.BAS - Sample app shows using BDPP to grab VDP buffer data |"
160 DATA "| This program does the following things:                              |"
170 DATA "| (1) Write text on the screen in various colors                       |"
180 DATA "| (2) Capture a portion of the screen into a buffer on the ESP32       |"
190 DATA "| (3) Setup BDPP RX packets on the EZ80                                |"
200 DATA "| (4) Tell the ESP32 to transmit the buffer contents to the EZ80       |"
210 DATA "| (5) Draw an image that is 1/4 the size of the captured image         |"
215 DATA "+----------------------------------------------------------------------+"
220 DATA ""
300 READ T$: IF T$="" GOTO 400
310 L%=LEN(T$)
320 FOR I%=1 TO L%
330 COLOUR RND(63): PRINT MID$(T$,I%,1);
340 NEXT I%: PRINT "": GOTO 300
400 REM
160 REM VDU 23,27,1,bufferId%,0,0;
170 REM VDU 23,0,&A0,bufferId%;&1B,packetIndex%,offsetLo%;offsetHi%;n%;
998 REM PRINT "Press ESC";
999 GOTO 999
