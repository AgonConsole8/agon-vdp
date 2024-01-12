10 REM This demo shows horizontal scrolling of a tile array.
20 REM MODE 32: REM 800x600x64 without text area
30 MODE 64: REM 800x600x64 with text area
105 T=TIME
106 IF TIME-T<100 THEN GOTO 106
100 FDFLT%=&000F: FHIDE%=&000E: FGRP%=&000E: FSAME%=&0200: FSCROLL1%=&0010: OTF%=&1E17
110 columns%=15: rows%=12
115 bmpWidth%=64: bmpHeight%=50: bmpSize%=bmpWidth%*bmpHeight%
116 rootID%=0: bmpID%=11: chunkLimit%=128: tileArrayID%=10
120 DIM A% bmpSize%
160 DIM code% 32
200 REM Create tile array
210 VDU OTF%; 80, tileArrayID%; rootID%; FHIDE%+FSCROLL1%; columns%; rows%; bmpWidth%; bmpHeight%; 800; 600;
220 PROC_LoadBmp("BITMAP_64X50_SOLID.BIN", A%, bmpSize%)
230 VDU OTF%;4,tileArrayID%;
240 FOR r%=0 TO rows%-1
250 FOR c%=0 TO columns%-1
260 VDU OTF%; 84, tileArrayID%; c%; r%; bmpID%;
265 PRINT "(";r%;",";c%;")";
270 NEXT c%
280 NEXT r%
300 VDU OTF%; 0, tileArrayID%; FDFLT%;
450 GOTO 450

10100 DEF PROC_LoadBmp(filename$, address%, totalBytes%)
10102 T=TIME
10104 IF TIME-T<25 THEN GOTO 10104
10110 cmd$="load "+filename$+" "+STR$(address%)
10120 PRINT cmd$;" (";~totalBytes%;" bytes as bitmap ID ";bmpID%;")"
10130 OSCLI cmd$
10140 VDU OTF%; 81, tileArrayID%; bmpID%; 0
10142 bytes%=totalBytes%: count%=0: M%=address%
10144 IF bytes%<=0 THEN GOTO 10160
10146 IF bytes%>chunkLimit% THEN N%=chunkLimit% ELSE N%=bytes%
10147 PROC_AsmSendChunk: *FX 19
10148 VDU OTF%; 88, tileArrayID%; bmpID%; count%; 0; N%;
10150 CALL code%
10151 T=TIME
10152 IF TIME-T<10 THEN GOTO 10152
10153 PRINT "  copied ";~N%;" from ";~M%;" to ";~count%;" of ";~totalBytes%
10154 bytes%=bytes%-N%: count%=count%+N%: M%=M%+N%
10155 GOTO 10144
10160 PRINT: PRINT "=== ";filename$;" sent ==="
10180 ENDPROC

10200 DEF PROC_AsmSendChunk
10210 P%=code%
10220 [OPT 2
10230 LD HL,M%
10240 LD BC,N%
10250 RST.LIL &18
10260 RET
10270 ]
10280 ENDPROC
