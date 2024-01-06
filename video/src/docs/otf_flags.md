<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

# OTF Primitve Flags

The OTF mode uses flags to control certain aspects of primitives.
Here is an overview of the current set of flags (which is always
subject to additions in the future):

```
#define PRIM_FLAG_PAINT_THIS  0x0001  // whether to paint this primitive
#define PRIM_FLAG_PAINT_KIDS  0x0002  // whether to paint child primitives
#define PRIM_FLAG_CLIP_THIS   0x0004  // whether to clip this primitive
#define PRIM_FLAG_CLIP_KIDS   0x0008  // whether to clip child primitives
#define PRIM_FLAG_H_SCROLL_1  0x0010  // whether to support horizontal scrolling on 1-pixel boundary
#define PRIM_FLAG_H_SCROLL_4  0x0020  // whether to support horizontal scrolling on 4-pixel boundary
#define PRIM_FLAG_ABSOLUTE    0x0040  // whether to use absolute coordinates always
#define PRIM_FLAGS_MASKED     0x0080  // hint that pixels are fully opaque or transparent
#define PRIM_FLAGS_BLENDED    0x0100  // hint that pixels may be blended
#define PRIM_FLAGS_ALL_SAME   0x0200  // hint that all lines can be drawn the same way
#define PRIM_FLAGS_LEFT_EDGE  0x0400  // hint that left edge of primitive may be cut off
#define PRIM_FLAGS_RIGHT_EDGE 0x0800  // hint that right edge of primitive may be cut off
#define PRIM_FLAGS_CAN_DRAW   0x1000  // whether this primitive can be drawn at all
#define PRIM_FLAGS_X          0x2000  // hint that x will be given
#define PRIM_FLAGS_X_SRC      0x4000  // hint that x and src pixel ptr will be given
#define PRIM_FLAGS_REF_DATA   0x8000  // whether this primitive references (vs owns) data
#define PRIM_FLAGS_DEFAULT    0x000F  // flags set when a new base primitive is constructed
#define PRIM_FLAGS_CHANGEABLE 0x000F  // flags that the app can change after primitive creation
```

The following describes the individual flags in more detail.

<b>PRIM_FLAG_PAINT_THIS</b><br>
This flag determines whether the primitive is painted (drawn).
It is essentially a <i>visibility</i> flag, because it partially
determines whether the primitive can be visible. Other factors,
such as position may also affect visibility. Inverting this flag
on a periodic basis will cause the primitive to blink. 

<b>PRIM_FLAG_PAINT_KIDS</b><br>
This flag determines whether this children of the primitive,
if any, are painted (drawn). This flag controls whether drawing
traverses lower levels of the primitive tree (nesting), below
the primitive itself. Note that even if this flag is set,
a child primitive will not be drawn if its own PRIM_FLAG_PAINT_THIS
flag is cleared. Inverting this flag on a periodic basis will
cause child primitives to blink.

<b>PRIM_FLAG_CLIP_THIS</b><br>
This flag determines whether this primitive is clipped by the draw
region of its parent. If this flag is cleared, the primitive
is clipped by the screen.
Presently, the draw region of any primitive
is defined when it is created, based on the designated size at
that time. There is currently no way to resize a primitive after
it has been created. It is possible to use a group primitive to
define a clipping region for child primitives. The group itself
has no visible aspect, but can affect its children. For example,
a group of spaceships can be moved together by moving their
parent group, if created as children of that group. It is possible
to simulate growing or shrinking by using multiple primitives,
and only displaying one at a time.

<b>PRIM_FLAG_CLIP_KIDS</b><br>
This flag determines whether to clip child primitives by the parent
primitive's draw region. If this flag is clear, the children are
clipped by their grandparent, if any, or by the screen, not by their parent.

<b>PRIM_FLAG_H_SCROLL_1</b><br>
This flag determines whether the primitive will need to be scrolled
horizontally on a 1-pixel boundary. This flag does not need to be
specified in order to scroll on a 4-pixel boundary (that flag is below).
If this flag is set when creating the primitive, more RAM will be used to
support the smooth horizontal motion. Using this flag implies being
able to scroll on a 1-, 2-, 3-, or 4-pixel boundary.
If no motion is necessary (i.e., the
primitive has just one location on the screen), then do not set
the PRIM_FLAG_H_SCROLL_1 flag or the PRIM_FLAG_H_SCROLL_4 flag.

NOTE: If any ancestor of the primitive uses this flag, then
this primitive must also use this flag.

<b>PRIM_FLAG_H_SCROLL_4</b><br>
This flag determines whether the primitive will need to be scrolled
horizontally on a 4-pixel boundary. If this flag
is set when creating the primitive, more RAM will be used to support
the rough horizontal motion. If no motion is necessary (i.e., the
primitive has just one location on the screen), then do not set
the PRIM_FLAG_H_SCROLL_1 flag or the PRIM_FLAG_H_SCROLL_4 flag.

NOTE: If any ancestor of the primitive uses this flag, then
this primitive must also use this flag.

<b>PRIM_FLAG_ABSOLUTE</b><br>
This flag determines whether the primitive should <i>always</i> be
positioned relative to the screen, and not to any of its ancestors.
If this flag is cleared, the displayed location of the primitive
may be affected by the locations of its parent, grandparent, etc.

<b>PRIM_FLAGS_MASKED</b><br>
This flag determines whether a primitive is intended to be drawn
in a masked manner, as opposed to a solid manner. In a solid, all
pixels are 100% opaque. In a mask, all pixels are either 100%
opaque or 0% opaque (100% transparent). This flag is used
for bitmaps, and helps the OTF mode to optimize drawing speed.

<b>PRIM_FLAGS_BLENDED</b><br>
This flag determines whether a primitive is intended to be drawn
in a blended manner, as opposed to a solid manner. In a solid, all
pixels are 100% opaque. In a blend, all pixels are either 100%,
75%, 50%, 25%, or 0% opaque (0%, 25%, 50%, 75%, or 100% transparent,
respectively). This flag is used
for bitmaps, and helps the OTF mode to optimize drawing speed. It
may also be used for other primitives such as lines and triangles.

<b>PRIM_FLAGS_ALL_SAME</b><br>
This flag determines whether all of the scan lines in the primitive
can be drawn using the same dynamic instructions. It is used for
rectangles and bitmaps, and helps the OTF mode to optimize drawing
speed, plus reduces the RAM used. It cannot be used with blending,
unless <i>all</i> pixels in the primitive are at the same opacity
level (e.g., all pixels are 50% opaque).

<b>PRIM_FLAGS_LEFT_EDGE</b><br>
This flag determines whether to generate code that can draw the
primitive with part of its left edge missing, such as hidden off
of the left side of the screen, <i>as a result of being moved
(scrolled)</i>. This flag must be set for any
primitive that will be <i>repositioned</i>
(after dynamic code generation for it)
such that its left edge is
hidden, either by being off-screen or clipped by some ancestor.
This flag is not required when part of the primitive is hidden
only by virtue of being underneath some other primitive, and is
not required for a static primitive that happens to be clipped
(meaning that part of it will never be drawn).

<b>PRIM_FLAGS_RIGHT_EDGE</b><br>
This flag determines whether to generate code that can draw the
primitive with part of its right edge missing, such as hidden off
of the right side of the screen, <i>as a result of being moved
(scrolled)</i>. This flag must be set for any
primitive that will be <i>repositioned</i>
(after dynamic code generation for it)
such that its right edge is
hidden, either by being off-screen or clipped by some ancestor.
This flag is not required when part of the primitive is hidden
only by virtue of being underneath some other primitive, and is
not required for a static primitive that happens to be clipped
(meaning that part of it will never be drawn).

<b>PRIM_FLAGS_CAN_DRAW</b><br>
This OTF-internal flag determines whether a primitive can be
drawn based on various factors, such as position, clipping, and
transparency. Its value is determined dynamically by OTF.

<b>PRIM_FLAGS_X</b><br>
This OTF-internal flag determines whether the X coordinate is provided,
as opposed to being fixed, when each scan line of a primitive
is drawn. This flag and some related code exist for potential future use.

<b>PRIM_FLAGS_X_SRC</b><br>
This OTF-internal flag determines whether the X coordinate is provided,
as opposed to being fixed, when each scan line of a primitive
is drawn. An example of where this is required is when drawing
a tile array or a tile map. OTF will set this flag automatically
in such cases, because it affects dynamic code generation.

<b>PRIM_FLAGS_REF_DATA</b><br>
This flag indicates that the primitive references underlying data
rather than owning the data. For example, the reference bitmap
primitives borrow pixel data from other bitmap primitives, but
do not own the bitmap data themselves.

<b>PRIM_FLAGS_DEFAULT</b><br>
This flag illustrates which flags are typically set when
creating a primitive. Most often, when creating a primitive,
such as a triangle, you would pass a value of 15 (0x000F)
as the flags parameter. You may want to use another value,
if you intend to create, but not immediately display,
the new primitive.

<b>PRIM_FLAGS_CHANGEABLE</b><br>
This flag illustrates which flags my be changed <i>after</i> the
primitive is created. Setting or clearing any of these flags
may have an affect on how or whether the primitive is drawn.

[Home](otf_mode.md)
