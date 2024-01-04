<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

## Create primitive: Solid Bitmap
<b>VDU 23, 30, 120, id; pid; flags; w; h; psram</b> : Create primitive: Solid Bitmap

This commmand creates a primitive that draws a solid bitmap, meaning
that every pixel is fully opaque (though each pixel has its own color).
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

OTF mode will automatically set the PRIM_FLAGS_ALL_SAME flag
when this command is used.

If the psram parameter is nonzero, the bitmap pixels are kept in PSRAM (SPI RAM)
rather than in DRAM. This should only be done in resolutions where the pixel clock
rate is relatively slow; otherwise, flicker may occur.

## Create primitive: Masked Bitmap
<b>VDU 23, 30, 121, id; pid; flags; w; h; psram, color</b> : Create primitive: Masked Bitmap

This commmand creates a primitive that draws a masked bitmap, meaning
that every pixel is either fully opaque (though each pixel has its own color) or fully transparent.
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

If the psram parameter is nonzero, the bitmap pixels are kept in PSRAM (SPI RAM)
rather than in DRAM. This should only be done in resolutions where the pixel clock
rate is relatively slow; otherwise, flicker may occur.

## Create primitive: Transparent Bitmap
<b>VDU 23, 30, 122, id; pid; flags; w; h; psram, color</b> : Create primitive: Transparent Bitmap

This commmand creates a primitive that draws a transparent bitmap, meaning
that each pixel has either 0%, 25%, 50%, 75%, or 100% opacity.
A transparent bitmap may be the least efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower than solid bitmaps, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

OTF mode will automatically set the PRIM_FLAGS_BLENDED flag
when this command is used.

If the psram parameter is nonzero, the bitmap pixels are kept in PSRAM (SPI RAM)
rather than in DRAM. This should only be done in resolutions where the pixel clock
rate is relatively slow; otherwise, flicker may occur.

## Set position & slice solid bitmap
<b>VDU 23, 30, 123, id; x; y; s; h;</b> : Set position & slice solid bitmap

This command sets the position of a solid bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set position & slice masked bitmap
<b>VDU 23, 30, 124, id; x; y; s; h;</b> : Set position & slice masked bitmap

This command sets the position of a masked bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set position & slice transparent bitmap
<b>VDU 23, 30, 125, id; x; y; s; h;</b> : Set position & slice transparent bitmap

This command sets the position of a transparent bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice solid bitmap
<b>VDU 23, 30, 126, id; x; y; s; h;</b> : Adjust position & slice solid bitmap

This command adjusts the position of a solid bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice masked bitmap
<b>VDU 23, 30, 127, id; x; y; s; h;</b> : Adjust position & slice masked bitmap

This command adjusts the position of a masked bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice transparent bitmap
<b>VDU 23, 30, 128, id; x; y; s; h;</b> : Adjust position & slice transparent bitmap

This command adjusts the position of a transparent bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set solid bitmap pixel
<b>VDU 23, 30, 129, id; x; y; color</b> : Set solid bitmap pixel

This command sets the color of a single pixel within a solid bitmap.
You must use the same alpha bits for every pixel in the bitmap;
otherwise, undesirable results may occur.

## Set masked bitmap pixel
<b>VDU 23, 30, 130, id; x; y; color</b> : Set masked bitmap pixel

This command sets the color of a single pixel within a masked bitmap.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap. All other pixels must have equal
alpha bits (i.e., be at the same opacity level).

## Set transparent bitmap pixel
<b>VDU 23, 30, 131, id; x; y; color</b> : Set transparent bitmap pixel

This command sets the color of a single pixel within a transparent bitmap.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

## Set solid bitmap pixels
<b>VDU 23, 30, 132, id; x; y; n; c0, c1, c2, ...</b> : Set solid bitmap pixels

This command sets the colors of multiple pixels within a solid bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.

The "n" parameter is the number of pixels.

## Set masked bitmap pixels
<b>VDU 23, 30, 133, id; x; y; n; c0, c1, c2, ...</b> : Set masked bitmap pixels

This command sets the colors of multiple pixels within a masked bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

The "n" parameter is the number of pixels.

## Set transparent bitmap pixels
<b>VDU 23, 30, 134, id; x; y; n; c0, c1, c2, ...</b> : Set transparent bitmap pixels

This command sets the colors of multiple pixels within a transparent bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

The "n" parameter is the number of pixels.

## Create primitive: Reference Solid Bitmap
<b>VDU 23, 30, 135, id; pid; flags; bmid;</b> : Create primitive: Reference Solid Bitmap

This commmand creates a primitive that references a solid bitmap.
The new primitive has its own position and slicing information, but has no
pixel data. It borrows (shares) the pixel data from the
referenced bitmap. This makes it possible to display multiple
similar bitmaps, using only one set of pixel data.

This command sets the PRIM_FLAGS_REF_DATA flag for the new primitive automatically.

## Create primitive: Reference Masked Bitmap
<b>VDU 23, 30, 136, id; pid; flags; bmid;</b> : Create primitive: Reference Masked Bitmap

This commmand creates a primitive that references a solid bitmap.
The new primitive has its own position and slicing information, but has no
pixel data. It borrows (shares) the pixel data from the
referenced bitmap. This makes it possible to display multiple
similar bitmaps, using only one set of pixel data.

This command sets the PRIM_FLAGS_REF_DATA flag for the new primitive automatically.

## Create primitive: Reference Transparent Bitmap
<b>VDU 23, 30, 137, id; pid; flags; bmid;</b> : Create primitive: Reference Transparent Bitmap

This commmand creates a primitive that references a solid bitmap.
The new primitive has its own position and slicing information, but has no
pixel data. It borrows (shares) the pixel data from the
referenced bitmap. This makes it possible to display multiple
similar bitmaps, using only one set of pixel data.

This command sets the PRIM_FLAGS_REF_DATA flag for the new primitive automatically.

The following image illustrates the concepts, but the actual appearances will differ on the Agon, because this image was created on a PC.

![Bitmap](bitmap.png)

[Home](otf_mode.md)
