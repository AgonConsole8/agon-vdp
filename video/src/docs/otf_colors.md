<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

# OTF Colors

The OTF mode can display all 64 colors that the Agon is capable
of showing. Since the OTF mode supports transparency, it uses
the upper 2 bits of the color byte for that. In other words,
it uses an AABBGGRR format, where the 8 bits of the color byte
are concerned. The alpha (opacity) values are as follows:

00BBGGRR - the pixel is 25% opaque (75% transparent)
01BBGGRR - the pixel is 50% opaque (50% transparent)
10BBGGRR - the pixel is 75% opaque (25% transparent)
11BBGGRR - the pixel is 100% opaque (0% transparent)

The obvious question is, how do we specify a pixel that is 0%
opaque (100% transparent)? That is done by choosing a byte value
that will not be used in the primitive to represent a color. If
the primitive has a consistent opaqueness (all visible pixels are at
the same opacity level), then the chosen byte value can be any value
that does not specify that particular opacity level. For example,
if all pixels are supposed to be 75% opaque (alpha 10 in binary),
then the value 0 (0x00) works just fine as the transparent
color, because it does not indicate 75% opacity.

The potential conflict arises when the primitive, such as a
bitmap, is intended to have variable blending, meaning that each
pixel can have its own opacity level. In that case, one of the
255 possible byte values must be sacrificed, and used as the
fully transparent color. Simply put, you must choose a value
that does not exist in the source bitmap. Bear in mind that the
<i>whole</i> byte value is used to represent an invisible pixel,
not just the color bits. This means that the transparent color
can have color bits equal to an actual color in the primitive,
but it must have different alpha bits, in that case. For example,
if 0x7F (bright white at 50% opacity) is a color in the source
bitmap, you cannot use 0x7F as the transparent color, but you
could use 0xFF, if there will be no bright white pixels at
100% opacity.

[Home](otf_mode.md)
