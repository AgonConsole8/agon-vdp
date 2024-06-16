# 3D Rendering via Pingo on Agon

## Introduction to 3D Render Commands

A 3D scene is composed of one or more object models (meshes), each of which uses
some texture for colorizing the model. A texture uses a bitmap for its
pixel colors, and the model uses texture coordinates to assign colors to its
surfaces when the render is performed.

There are various mathematical computations involved in 3D rendering, of course,
and floating point numbers are used for many of those computations. However, VDU
commands do not directly support passing floating point numbers, as the VDU statement
only supports passing 1-byte and 2-byte values. For that reason, many of the
values passed to the render commands are scaled values.

The commands below use numbers with the following meaning and ranges:
<br><br><b>sid</b>: A VDU buffer ID that acts as a scene ID. It refers to a control structure for the 3D scene.
<br><br><b>mid</b>: A specific mesh ID in the range 0 to 65535.
<br><br><b>oid</b>: A specific object ID in the range 0 to 65535.
<br><br><b>n</b>: A positive number (count) of things that follow within the same command.
<br><br><b>x</b>: A 2D X coordinate in the range -32768 to +32767. Often, the value is in or near the range of 0 to screen width.
<br><br><b>y</b>: A 2D Y coordinate in the range -32768 to +32767. Often, the value is in or near the range of 0 to screen height.
<br><br><b>w</b>: A positive width that typically ranges from 1 to screen width.
<br><br><b>h</b>: A positive height that typically ranges from 1 to screen height.
<br><br><b>x0, y0, z0</b>: A prescaled 3D X, Y, or Z coordinate in the range -32767 to +32767.
This number is divided by 32767 to yield a floating point number in the range -1.0 to +1.0, for 3D computations.
To prescale a set of coordinates for use in a VDU command, scale them all to fit within the range -1.0 to +1.0,
then multiply the new floating point values by 32767.
```
F = FACTOR * 32767
PX = X * F
PY = Y * F
PZ = Z * F
VDU ... PX; PY; PZ; ...
```
<br><br><b>i0</b>: A zero-based index into a list of coordinates (mesh or texture).
<br><br><b>u0, v0</b>: A texture coordinate ranging from 0 to 65535.
This value is divided by 65535 to yield a floating point number, for 3D computations.
To prescale a set of coordinates for use in a VDU command, scale them all to fit within the range 0.0 to +1.0,
then multiply the new floating point values by 65535, as appropriate.
```
PU = U * HORIZ_FACTOR * 65535
PV = V * VERT_FACTOR * 65535
VDU ... PU; PV; ...
```
<br><br><b>scalex, scaley, scalez</b>: A prescaled 3D X, Y, or Z scale value in the range 0 to 65535.
This number is divided by 256 to yield a floating point number in the approximate range 0.0 to near 256.0, for 3D computations.
To prescale a set of scale factors for use in a VDU command, multiply them by 256.
```
F = 256
PSX = SX * F
PSY = SY * F
PSZ = SZ * F
VDU ... PSX; PSY; PSZ; ...
```
<br><br><b>anglex, angley, anglez</b>: A prescaled 3D X, Y, or Z rotation angle value in the range -32767 to +32767.
This number is divided by 32767 to yield a floating point number in the range -1.0 to +1.0, for 3D computations.
The resulting number is multiplied by 2PI, to yield an angle in radians.
Thus, the passed value of -32767 means -2PI, and +32767 means +2PI.
To prescale a set of angles in radians for use in a VDU command, divide the angles by 2PI, which will
scale them all to fit within the range -1.0 to +1.0,
then multiply the new floating point values by 32767.
```
F = 32767 / TWOPI
PAX = AX * F
PAY = AY * F
PAZ = AZ * F
VDU ... PAX; PAY; PAZ; ...
```
<br><br><b>distx, disty, distz</b>: A prescaled 3D X, Y, or Z translation distance in the range -32767 to +32767.
This number is divided by 32767 to yield a floating point number in the range -1.0 to +1.0, for 3D computations.
The resulting number is multiplied by 256.0.
Thus, the passed value of -32767 means -256.0, and +32767 means +256.0.
To prescale a set of distances for use in a VDU command, divide the distances by 256,
scale them all to fit within the range -1.0 to +1.0,
then multiply the new floating point values by 32767.
```
F = 32767 / 256
PDX = DX * F
PDY = DY * F
PDZ = DZ * F
VDU ... PDX; PDY; PDZ; ...
```
<br>

# VDU Commands for 3D Rendering

## Overview of Commands

<b>VDU 23, 0, &A0, sid; &48, 0, w; h;</b> :  Create Control Structure<br>
<b>VDU 23, 0, &A0, sid; &48, 1, mid; n; x0; y0; z0; ...</b> :  Define Mesh Vertices<br>
<b>VDU 23, 0, &A0, sid; &48, 2, mid; n; i0; ...</b> :  Set Mesh Vertex Indexes<br>
<b>VDU 23, 0, &A0, sid; &48, 3, mid; n; u0; v0; ...</b> :  Define Texture Coordinates<br>
<b>VDU 23, 0, &A0, sid; &48, 4, mid; n; i0; ...</b> :  Set Texture Coordinate Indexes<br>
<b>VDU 23, 0, &A0, sid; &48, 5, oid; mid; bmid;</b> :  Create Object<br>
<b>VDU 23, 0, &A0, sid; &48, 6, oid; scalex;</b> :  Set Object X Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 7, oid; scaley;</b> :  Set Object Y Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 8, oid; scalez;</b> :  Set Object Z Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 9, oid; scalex; scaley; scalez;</b> :  Set Object XYZ Scale Factors<br>
<b>VDU 23, 0, &A0, sid; &48, 10, oid; anglex;</b> :  Set Object X Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 11, oid; angley;</b> :  Set Object Y Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 12, oid; anglez;</b> :  Set Object Z Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 13, oid; anglex; angley; anglez;</b> :  Set Object XYZ Rotation Angles<br>
<b>VDU 23, 0, &A0, sid; &48, 14, oid; distx;</b> :  Set Object X Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 15, oid; disty;</b> :  Set Object Y Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 16, oid; distz;</b> :  Set Object Z Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 17, oid; distx; disty; distz;</b> :  Set Object XYZ Translation Distances<br>
<b>VDU 23, 0, &A0, sid; &48, 18, anglex;</b> :  Set Camera X Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 19, angley;</b> :  Set Camera Y Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 20, anglez;</b> :  Set Camera Z Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 21, anglex; angley; anglez;</b> :  Set Camera XYZ Rotation Angles<br>
<b>VDU 23, 0, &A0, sid; &48, 22, distx;</b> :  Set Camera X Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 23, disty;</b> :  Set Camera Y Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 24, distz;</b> :  Set Camera Z Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 25, distx; disty; distz;</b> :  Set Camera XYZ Translation Distances<br>
<b>VDU 23, 0, &A0, sid; &48, 26, scalex;</b> :  Set Scene X Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 27, scaley;</b> :  Set Scene Y Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 28, scalez;</b> :  Set Scene Z Scale Factor<br>
<b>VDU 23, 0, &A0, sid; &48, 29, scalex; scaley; scalez;</b> :  Set Scene XYZ Scale Factors<br>
<b>VDU 23, 0, &A0, sid; &48, 30, anglex;</b> :  Set Scene X Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 31, angley;</b> :  Set Scene Y Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 32, anglez;</b> :  Set Scene Z Rotation Angle<br>
<b>VDU 23, 0, &A0, sid; &48, 33, anglex; angley; anglez;</b> :  Set Scene XYZ Rotation Angles<br>
<b>VDU 23, 0, &A0, sid; &48, 34, distx;</b> :  Set Scene X Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 35, disty;</b> :  Set Scene Y Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 36, distz;</b> :  Set Scene Z Translation Distance<br>
<b>VDU 23, 0, &A0, sid; &48, 37, distx; disty; distz;</b> :  Set Scene XYZ Translation Distances<br>
<b>VDU 23, 0, &A0, sid; &48, 38, bmid;</b> :  Render To Bitmap<br>
<b>VDU 23, 0, &A0, sid; &48, 39</b> :  Delete Control Structure<br>

## Create Control Structure
<b>VDU 23, 0, &A0, sid; &48, 0, w; h;</b> :  Create Control Structure<br>

This command initializes a control structure used to
do 3D rendering. The structure is housed inside the designated buffer. The buffer
referred to by the scene ID (sid) is created, if it does not already exist.

The given width and height determine the size of the final rendered scene.

## Define Mesh Vertices
<b>VDU 23, 0, &A0, sid; &48, 1, mid; n; x0; y0; z0; ...</b> :  Define Mesh Vertices

This command establishes the list of mesh coordinates to be used to define
a surface structure. The mesh may be referenced by multiple objects.

The "n" parameter is the number of vertices, so the total number of coordinates specified equals n*3.

## Set Mesh Vertex Indexes
<b>VDU 23, 0, &A0, sid; &48, 2, mid; n; i0; ...</b> :  Set Mesh Vertex Indexes

This command lists the indexes of the vertices that define a 3D mesh. Individual
vertices are often referenced multiple times within a mesh, because they are
often part of multiple surface triangles. Each index value ranges from 0 to
the number of defined mesh vertices.

The "n" parameter is the number of indexes, and must match the "n" in subcommand 4
(Set Texture Coordinate Indexes).

## Define Texture Coordinates
<b>VDU 23, 0, &A0, sid; &48, 3, mid; n; u0; v0; ...</b> :  Define Texture Coordinates

This command establishes the list of U/V texture coordinates that define texturing
for a mesh.

The "n" parameter is the number of coordinate pairs, so the total number of coordinates specified equals n*2.

## Set Texture Coordinate Indexes
<b>VDU 23, 0, &A0, sid; &48, 4, mid; n; i0; ...</b> :  Set Texture Coordinate Indexes

This command lists the indexes of the coordinates that define a 3D texture for a mesh.
Individual coordinates may be referenced multiple times within a texture,
but that is not required. The number of indexes passed in this command must match
the number of mesh indexes defining the mesh. Thus, each mesh vertex has texture
coordinates associated with it.

The "n" parameter is the number of indexes, and must match the "n" in subcommand 2
(Set Mesh Vertex Indexes).

## Define Object
<b>VDU 23, 0, &A0, sid; &48, 5, oid; mid; bmid;</b> :  Create Object

This command defines a renderable object in terms of its already-defined mesh,
plus a reference to an existing bitmap that provides its coloring, via the
texture coordinates used by the mesh. The same mesh can be used multiple times,
with the same or different bitmaps for coloring. The bitmap must be in the
RGBA8888 format (4 bytes per pixel).

## Set Object X Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 6, oid; scalex;</b> :  Set Object X Scale Factor

This command sets the X scale factor for an object.

## Set Object Y Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 7, oid; scaley;</b> :  Set Object Y Scale Factor

This command sets the Y scale factor for an object.

## Set Object Z Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 8, oid; scalez;</b> :  Set Object Z Scale Factor

This command sets the Z scale factor for an object.

## Set Object XYZ Scale Factors
<b>VDU 23, 0, &A0, sid; &48, 9, oid; scalex; scaley; scalez;</b> :  Set Object XYZ Scale Factors

This command sets the X, Y, and Z scale factors for an object.

## Set Object X Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 10, oid; anglex;</b> :  Set Object X Rotation Angle

This command sets the X rotation angle for an object.

## Set Object Y Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 11, oid; angley;</b> :  Set Object Y Rotation Angle

This command sets the Y rotation angle for an object.

## Set Object Z Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 12, oid; anglez;</b> :  Set Object Z Rotation Angle

This command sets the Z rotation angle for an object.

## Set Object XYZ Rotation Angles
<b>VDU 23, 0, &A0, sid; &48, 13, oid; anglex; angley; anglez;</b> :  Set Object XYZ Rotation Angles

This command sets the X, Y, and Z rotation angles for an object.

## Set Object X Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 14, oid; distx;</b> :  Set Object X Translation Distance

This command sets the X translation distance for an object.
Note that 3D translation of an object is independent of 2D translation of the the rendered bitmap.

## Set Object Y Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 15, oid; disty;</b> :  Set Object Y Translation Distance

This command sets the Y translation distance for an object.
Note that 3D translation of an object is independent of 2D translation of the the rendered bitmap.

## Set Object Z Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 16, oid; distz;</b> :  Set Object Z Translation Distance

This command sets the Z translation distance for an object.
Note that 3D translation of an object is independent of 2D translation of the the rendered bitmap.

## Set Object XYZ Translation Distances
<b>VDU 23, 0, &A0, sid; &48, 17, oid; distx; disty; distz;</b> :  Set Object XYZ Translation Distances

This command sets the X, Y, and Z translation distances for an object.
Note that 3D translation of an object is independent of 2D translation of the the rendered bitmap.

## Set Camera X Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 18, anglex;</b> :  Set Camera X Rotation Angle

This command sets the X rotation angle for the camera.

## Set Camera Y Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 19, angley;</b> :  Set Camera Y Rotation Angle

This command sets the Y rotation angle for the camera.

## Set Camera Z Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 20, anglez;</b> :  Set Camera Z Rotation Angle

This command sets the Z rotation angle for the camera.

## Set Camera XYZ Rotation Angles
<b>VDU 23, 0, &A0, sid; &48, 21, anglex; angley; anglez;</b> :  Set Camera XYZ Rotation Angles

This command sets the X, Y, and Z rotation angles for the camera.

## Set Camera X Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 22, distx;</b> :  Set Camera X Translation Distance

This command sets the X translation distance for the camera.
Note that 3D translation of the camera is independent of 2D translation of the the rendered bitmap.

## Set Camera Y Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 23, disty;</b> :  Set Camera Y Translation Distance

This command sets the Y translation distance for the camera.
Note that 3D translation of the camera is independent of 2D translation of the the rendered bitmap.

## Set Camera Z Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 24, distz;</b> :  Set Camera Z Translation Distance

This command sets the Z translation distance for the camera.
Note that 3D translation of the camera is independent of 2D translation of the the rendered bitmap.

## Set Camera XYZ Translation Distances
<b>VDU 23, 0, &A0, sid; &48, 25, distx; disty; distz;</b> :  Set Camera XYZ Translation Distances

This command sets the X, Y, and Z translation distances for the camera.
Note that 3D translation of the camera is independent of 2D translation of the the rendered bitmap.

## Set Scene X Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 26, scalex;</b> :  Set Scene X Scale Factor

This command sets the X scale factor for the scene.

## Set Scene Y Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 27, scaley;</b> :  Set Scene Y Scale Factor

This command sets the Y scale factor for the scene.

## Set Scene Z Scale Factor
<b>VDU 23, 0, &A0, sid; &48, 28, scalez;</b> :  Set Scene Z Scale Factor

This command sets the Z scale factor for the scene.

## Set Scene XYZ Scale Factors
<b>VDU 23, 0, &A0, sid; &48, 29, scalex; scaley; scalez;</b> :  Set Scene XYZ Scale Factors

This command sets the X, Y, and Z scale factors for the scene.

## Set Scene X Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 30, anglex;</b> :  Set Scene X Rotation Angle

This command sets the X rotation angle for the scene.

## Set Scene Y Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 31, angley;</b> :  Set Scene Y Rotation Angle

This command sets the Y rotation angle for the scene.

## Set Scene Z Rotation Angle
<b>VDU 23, 0, &A0, sid; &48, 32, anglez;</b> :  Set Scene Z Rotation Angle

This command sets the Z rotation angle for the scene.

## Set Scene XYZ Rotation Angles
<b>VDU 23, 0, &A0, sid; &48, 33, anglex; angley; anglez;</b> :  Set Scene XYZ Rotation Angles

This command sets the X, Y, and Z rotation angles for the scene.

## Set Scene X Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 34, distx;</b> :  Set Scene X Translation Distance

This command sets the X translation distance for the scene.
Note that 3D translation of the scene is independent of 2D translation of the the rendered bitmap.

## Set Scene Y Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 35, disty;</b> :  Set Scene Y Translation Distance

This command sets the Y translation distance for the scene.
Note that 3D translation of the scene is independent of 2D translation of the the rendered bitmap.

## Set Scene Z Translation Distance
<b>VDU 23, 0, &A0, sid; &48, 36, distz;</b> :  Set Scene Z Translation Distance

This command sets the Z translation distance for the scene.
Note that 3D translation of the scene is independent of 2D translation of the the rendered bitmap.

## Set Scene XYZ Translation Distances
<b>VDU 23, 0, &A0, sid; &48, 37, distx; disty; distz;</b> :  Set Scene XYZ Translation Distances

This command sets the X, Y, and Z translation distances for the scene.
Note that 3D translation of the scene is independent of 2D translation of the the rendered bitmap.

## Render To Bitmap
<b>VDU 23, 0, &A0, sid; &48, 38, bmid;</b> :  Render To Bitmap

This command uses information provided by the above commands to render the 3D scene
onto the specified bitmap. This command must be used in
order to perform the render operation; it does <i>not</i> happen automatically, when other
commands change some of the render parameters.

## Delete Control Structure
<b>VDU 23, 0, &A0, sid; &48, 39</b> :  Delete Control Structure<br>

This command deinitializes an existing control structure,
assuming that it exists in the designated buffer. The buffer is subsequently
deleted, as part of processing for this command.

## Sample

The following image illustrates the concept.

![Render](render.png)
