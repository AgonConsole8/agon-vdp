10 REM oscli_rst.bas
20 REM Sample showing loading and sending bulk data to VDP.
30 REM This app will list itself to the screen.
40 REM On Linux, it only has LF, not CRLF, so the listing shows that.
100 DIM text% 750: REM # of bytes for load area
105 PRINT "Text load area at &"; ~text%
110 DIM code% 20: REM # of bytes for assembler area
115 PRINT "Assembler area at &"; ~code%
120 OSCLI "LOAD oscli_rst.bas " + STR$(text%)
130 size%=703: REM # of characters in this program
160 PROC_assemble_rst
170 CALL code%
190 END
50000 DEF PROC_assemble_rst
50010 P%=code%
50020 [OPT 2
50030 LD BC,size%
50040 LD HL,text%
50050 RST.LIL &18
50060 RET
50070 ]
50080 REM PRINT "total code: ";P%-code%
50090 ENDPROC
