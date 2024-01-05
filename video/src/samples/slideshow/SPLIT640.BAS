10 REM OTF Split Image Demo
20 REM This demo shows an image split between internal and external RAM, using On-the-Fly mode.
30 MODE &44: REM OTF 640x480x64, with 80x60 Text Area
40 CLS: VDU 23,1,0;0;0;0;
50 FDFLT%=&000F
60 OTF%=&1E17
62 width%=640/4: height%=480/4
70 REM -- Load bitmaps
80 FOR ID%=100 TO 115
85 IF (ID% AND 3)=0 THEN usePSRAM%=0 ELSE usePSRAM%=0
90 VDU OTF%;120,ID%;0;FDFLT%;width%;height%;usePSRAM%
100 NEXT ID%
110 READ filename$
120 PRINT "Loading ";filename$
130 infile=OPENIN filename$
140 IF infile=0 THEN END
150 FOR trow%=0 TO 3
160 FOR line%=0 TO height%-1
170 FOR tcol%=0 TO 3
180 ID%=trow%*4+tcol%+100
185 PRINT "R:";trow%;" L:";line%;" C:";tcol%;" ID:";ID%
190 VDU OTF%;132,ID%;0;line%;width%;
200 FOR X%=0 TO width%-1
210  temp%=BGET#infile
220  VDU temp%+&C0
230 NEXT X%
235 NEXT tcol%
240 NEXT line%
245 NEXT trow%
250 CLOSE#infile

260 FOR trow%=0 TO 3
270 Y%=trow%*height%
280 FOR tcol%=0 TO 3
290 ID%=trow%*4+tcol%+100
292 X%=tcol%*width%
294 PRINT "R:";trow%;" Y:";Y%;" C:";tcol%;" X:";X%
300 VDU OTF%;1,ID%;X%;Y%;
310 VDU OTF%;4,ID%;
320 NEXT tcol%
330 NEXT trow%
335 T=TIME
336 IF TIME-T<500 THEN GOTO 335
340 VDU OTF%;3,1;: REM Delete text area
350 GOTO 350
299 REM --------
302 DATA "SLIDE01_640x480.BIN"
