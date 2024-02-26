<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

## Set flags for primitive
<b>VDU 23, 30, 0, id; flags;</b> :  Set flags for primitive

This command modifies certain flag bits for a primitive. Some flag bits are fixed upon creation
of the primitive, and cannot be changed. Refer to the OTF Primitive
Flags section, above, for more information.

## Set primitive position
<b>VDU 23, 30, 1, id; x; y;</b> :  Set primitive position

If a primitive is not using the PRIM_FLAG_ABSOLUTE flag, then
this command sets the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), then setting the primitive's position to (5, 5) will
place the primitive at position (105, 105), relative to the primitive's grandparent (if any),
or in other words, at a 5-pixel offset from its parent, in X and Y.

If a primitive is using the PRIM_FLAG_ABSOLUTE flag, then this command
sets the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Adjust primitive position
<b>VDU 23, 30, 2, id; x; y;</b> :  Adjust primitive position

If a primitive is not using the PRIM_FLAG_ABSOLUTE flag, then
this command ajusts the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), and the primitive is at (5, 5) then adjusting the primitive's position by (2, 2) will
place the primitive at position (107, 107), relative to the primitive's grandparent (if any),
or in other words, at a 7-pixel offset from its parent, in X and Y.

If a primitive is using the PRIM_FLAG_ABSOLUTE flag, then this command
adjusts the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Delete primitive
<b>VDU 23, 30, 3, id;</b> :  Delete primitive

This command deletes the primitive, and any children that it has. If
the primitive, such as a tile map, holds its own bitmaps, those will be
deleted with the primitive. Bear in mind that you can hide a
primitive by changing its flags, while still keeping it intact.

## Generate code for primitive
<b>VDU 23, 30, 4, id;</b> :  Generate code for primitive

This command generates dynamic code for the primitive, and any children that it has.
This command must be used after creating the primitive, and if needed, configuring
it, before the primitive will be drawn. For example, after creating a tile map and
its child bitmaps, use this command to generate code that can draw the tile map.

[Home](otf_mode.md)
