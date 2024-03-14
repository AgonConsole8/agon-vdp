10 REM
20 REM
30 REM
100 bufferId%=9
110 packetIndex%=0
120 offsetLo%=0: offsetHi%=0
130 n%=256
140 MODE 3
142 PRINT "Now in MODE 3"
145 GCOL 0,1
150 LINE 10,15,75,105
155 PRINT "See line?"
160 REM VDU 23,27,1,bufferId%,0,0;
170 REM VDU 23,0,&A0,bufferId%;&1B,packetIndex%,offsetLo%;offsetHi%;n%;
998 REM PRINT "Press ESC";
999 GOTO 999
