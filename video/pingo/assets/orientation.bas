   10 REM orientation.bas
   20 REM Sample app to show Pingo 3D axes on Agon
   30 REM
   40 REM -- VERTICES --
   50 arrow_vertices%=14
   60 DATA -1,0.1,-0.1, 0.5,0.1,-0.1
   70 DATA 0.5,0.2,-0.1, 1.0,0.0,-0.1
   80 DATA 0.5,-0.2,-0.2, 0.5,-0.1,-0.1
   90 DATA -1.0,-0.1,-0.2
  100 DATA -1,0.1,0.1, 0.5,0.1,0.1
  110 DATA 0.5,0.2,0.1, 1.0,0.0,0.1
  120 DATA 0.5,-0.2,0.2, 0.5,-0.1,0.1
  130 DATA -1.0,-0.1,0.2
 4380 REM
 4390 REM -- VERTEX INDEXES --
 4400 DATA 0,1,5, 5,6,0, 1,2,3, 3,4,5, 5,1,3
 4410 DATA 8,7,12, 12,7,13, 9,8,10, 10,12,11, 12,10,8
 4420 DATA 13,7,6, 7,0,6, 7,8,0, 0,8,1
 4430 DATA 13,6,12, 6,5,12, 8,9,2, 8,2,1
 4440 DATA 11,12,5, 11,5,4, 2,9,10, 3,2,10
 4450 DATA 3,11,4, 3,10,11
 6130 REM
 6140 REM -- CODE --
 6150 PRINT "Reading vertices"
 6160 total_coords%=arrow_vertices%*3
 6165 total_indexes%=24*3
 6170 max_abs=-99999
 6180 DIM vertices(total_coords%)
 6190 FOR i%=0 TO total_coords%-1
 6200   READ coord
 6210   vertices(i%)=coord
 6220   a=ABS(coord)
 6230   IF a>max_abs THEN max_abs=a
 6240 NEXT i%
 6250 factor=32767.0/max_abs
 6260 PRINT "Max absolute value = ";max_abs
 6270 PRINT "Factor = ";factor
 6280 sid%=100: mid1%=1: oid1%=1: oid2%=2: oid3%=3
 6282 bmid1%=101: bmid2%=102: bmid3%=103: bmido%=104
 6290 PRINT "Creating control structure"
 6300 scene_width%=200: scene_height%=200
 6310 VDU 23,0, &A0, sid%; &49, 0, scene_width%; scene_height%; : REM Create Control Structure
 6320 f=32767.0/256.0
 6330 distx=0.6*f: disty=2*f: distz=-25*f
 6340 VDU 23,0, &A0, sid%; &49, 25, distx; disty; distz; : REM Set Camera XYZ Translation Distances
 6350 pi2=PI*2.0: f=32767.0/pi2
 6360 anglex=-0.4*f
 6370 VDU 23,0, &A0, sid%; &49, 18, anglex; : REM Set Camera X Rotation Angle
 6380 PRINT "Sending vertices using factor ";factor
 6390 VDU 23,0, &A0, sid%; &49, 1, mid1%; arrow_vertices%; : REM Define Mesh Vertices
 6400 FOR i%=0 TO total_coords%-1
 6410   val%=vertices(i%)*factor
 6420   VDU val%;
 6430   T%=TIME
 6440   IF TIME-T%<1 GOTO 6440
 6450 NEXT i%
 6460 PRINT "Sending vertex indexes"
 6470 VDU 23,0, &A0, sid%; &49, 2, mid1%; total_indexes%; : REM Set Mesh Vertex Indexes
 6480 FOR i%=0 TO total_indexes%-1
 6490   READ val%
 6500   VDU val%;
 6510   T%=TIME
 6520   IF TIME-T%<1 GOTO 6520
 6530 NEXT i%
 6540 PRINT "Sending texture coordinates"
 6550 VDU 23,0, &A0, sid%; &49, 3, mid1%; arrow_vertices%; : REM Define Mesh Texture Coordinates
 6552 FOR i%=0 TO arrow_vertices%-1
 6554   VDU 0;
 6556   VDU 0;
 6558 NEXT i%
 6560 PRINT "Sending texture coordinate indexes"
 6562 RESTORE 4400
 6564 VDU 23,0, &A0, sid%; &49, 4, mid1%; total_indexes%; : REM Set Texture Coordinate Indexes
 6570 FOR i%=0 TO total_indexes%-1
 6575   READ val%
 6580   VDU val%;
 6590 NEXT i%
 6600 PRINT "Creating texture bitmap"
 6610 VDU 23, 27, 0, bmid1%: REM Create a bitmap for a texture
 6612 VDU 23, 27, 1, 1; 1; &FF, &00, &00, &C0 : REM Set a RED pixel in the bitmap
 6614 VDU 23, 27, 0, bmid2%: REM Create a bitmap for a texture
 6616 VDU 23, 27, 1, 1; 1; &00, &FF, &00, &C0 : REM Set a GREEN pixel in the bitmap
 6618 VDU 23, 27, 0, bmid3%: REM Create a bitmap for a texture
 6620 VDU 23, 27, 1, 1; 1; &00, &00, &FF, &C0 : REM Set a BLUE pixel in the bitmap
 6640 PRINT "Create 3D object"
 6650 VDU 23,0, &A0, sid%; &49, 5, oid1%; mid1%; bmid1%+64000; : REM Create Object
 6652 VDU 23,0, &A0, sid%; &49, 5, oid2%; mid1%; bmid2%+64000; : REM Create Object
 6654 VDU 23,0, &A0, sid%; &49, 5, oid3%; mid1%; bmid3%+64000; : REM Create Object
 6660 PRINT "Scale object"
 6670 scale=5.0*256.0
 6680 VDU 23, 0, &A0, sid%; &49, 9, oid1%; scale; scale; scale; : REM Set Object XYZ Scale Factors
 6682 VDU 23, 0, &A0, sid%; &49, 9, oid2%; scale; scale; scale; : REM Set Object XYZ Scale Factors
 6684 VDU 23, 0, &A0, sid%; &49, 9, oid3%; scale; scale; scale; : REM Set Object XYZ Scale Factors
 6690 PRINT "Create target bitmap"
 6700 VDU 23, 27, 0, bmido% : REM Select output bitmap
 6710 VDU 23, 27, 2, scene_width%; scene_height%; &0000; &00C0; : REM Create solid color bitmap
 6720 PRINT "Render 3D object"
 6730 VDU 23, 0, &C3: REM Flip buffer
 6740 rotatex=0.0: rotatey=0.0: rotatez=0.0
 6750 incx=PI/67.0: incy=PI/59.0: incz=PI/73.0
 6760 factor=32767.0/pi2
 6770 VDU 22, 136: REM 320x240x64
 6780 VDU 23, 0, &C0, 0: REM Normal coordinates
 6790 GCOL 0,136: CLG
 6810 rotatex=rotatex+incx: IF rotatex>=pi2 THEN rotatex=rotatex-pi2
 6820 rotatey=rotatey+incy: IF rotatey>=pi2 THEN rotatey=rotatey-pi2
 6830 rotatez=rotatez+incz: IF rotatez>=pi2 THEN rotatez=rotatez-pi2
 6840 rx=rotatex*factor: ry=rotatey*factor: rz=rotatez*factor
 6850 PRINT TAB(1,1) rx,rotatex
 6852 PRINT TAB(1,2) ry,rotatey
 6854 PRINT TAB(1,3) rz,rotatez
 6860 VDU 23, 0, &A0, sid%; &49, 10, oid1%; rx; : REM Set Object X Rotation Angle
 6870 VDU 23, 0, &A0, sid%; &49, 11, oid2%; ry; : REM Set Object Y Rotation Angle
 6880 VDU 23, 0, &A0, sid%; &49, 12, oid3%; rz; : REM Set Object Z Rotation Angle
 6890 VDU 23, 0, &A0, sid%; &49, 38, bmido%+64000; : REM Render To Bitmap
 6900 VDU 23, 27, 3, 320/2-200/2; 240/2-200/2; : REM Display output bitmap
 6910 VDU 23, 0, &C3: REM Flip buffer
 6920 *FX 19
 6930 GOTO 6790
