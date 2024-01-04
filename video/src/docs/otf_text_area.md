<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

## Create primitive: Text Area
<b>VDU 23, 30, 150, id; pid; flags; x; y; columns; rows; fgcolor, bgcolor</b> : Create primitive: Text Area

This command creates a text area primitive, which is a tile array
used to show text characters. The text area supports a flashing
cursor and cursor movement as characters are written, or as
directed by certain VDU commands.

You may create more than one text area. If you have no other full
background defined, you can use a full-screen text area as the
background, by creating it with 100 columns and 75 rows,
resulting in 800x600 pixel coverage.

At present, the only font available for a text area is the
built-in Agon system font (8x8 pixel characters), so there
is no font specified in this command. If other fonts are used
in the future, a new command will be added to the OTF mode.

When a text area is created, the OTF manager also creates a cursor
for it, which is a child primitive to the text area, with an ID
equal to the text area's ID plus one. Be sure not to use the ID
of the cursor for some other new primitive, or the cursor will
not function properly, and your new primitive may get moved
as text area characters are displayed.

The given foreground and background colors are set as the initial
current colors for the text area, so that when characters are printed,
they use those colors. The colors may be changed with the COLOUR statement
in BASIC, or with the proper VDU command.

The given foreground color is also used as the color of the blinking cursor,
which is a horizontal bar by default. In the future, it may be possible to
change the appearance of the cursor.

## Select Active Text Area
<b>VDU 23, 30, 151, id;</b> : Select Active Text Area

This command tells the OTF manager which text area should be the
active text area, when more than one text area was created. This
allows, for example, two separate areas of the screen to have
independent text (and possibly in the future, independent fonts).

## Define Text Area Character
<b>VDU 23, 30, 152, id; char, fgcolor, bgcolor</b> : Define Text Area Character

This command defines a single character to be used within a text area
primitive, meaning that it both creates a bitmap and sets the
initial pixel data of the bitmap, using the font that is
referenced by the text area.

It is possible to use multiple different colors for the same
character glyph, in a single text area, simply by defining
the character multiple times, using different colors.

## Define Text Area Character Range
<b>VDU 23, 30, 153, id; firstchar, lastchar, fgcolor, bgcolor</b> : Define Text Area Character Range

This command defines a range of characters to be used within a text area
primitive, meaning that it both creates the bitmaps and sets the
initial pixel data of the bitmaps, using the font that is
referenced by the text area.

It is possible to use multiple different colors for the same
character glyphs, in a single text area, simply by defining
the characters multiple times, using different colors.

[Home](otf_mode.md)
