## Issues and Considerations

The following OTF capabilities have not yet been implemented or are presently incomplete, and should not be used (yet):

* Drawing circles and ellipses.
* Doing 3D rendering.
* Using tile maps (you can use tile arrays).
* Using a mouse (because there is no support for the mouse pointer).
* Using some video modes, especially certain double-scan modes. You can try, but some pixels/characters may be cut off (not shown).
* Using sound. This has not been tested at all, and the high-priority OTF task may prevent it from working at present.
* Using very large bitmaps (from PSRAM) at resolutions above 320x240, because PSRAM is too slow to keep up with fast pixel clock rates. It is still possible to use smaller bitmaps at higher resolutions, such as 800x600.
* Once the video mode is changed to an OTF mode, there is no way to exit that mode without resetting the Agon. This will be fixed later.
* Partial transparency (as opposed to fully transparent or fully opaque) may not work (yet) in video modes with negative HS or VS signals. It should work in video modes where both signals are positive. This will be corrected soon.

The following things should be considered:

* OTF requires a different way of planning and construction an application from a displayable primitives perspective.
* OTF is not suitable for every kind of application, such as those that constantly draw on a full-screen buffer, purposely overwriting what was there.
* OTF is more suitable for applications that create various primitives and then manipulate (move, show, hide, delete) them.
* Where there is a need to move groups of objects together, consider using group primitives.
* Use group primitives to show/hide sets of objects with little serial communication (one command versus several), bearing in mind that the primitive flags need to be set correctly to support this.
* OTF may be a relatively simple idea, but its code is complex, so do not be surprised if you discover a latent bug.
* OTF is not currently hooked into all of the existing video-related BASIC statements (or their corresponding VDU commands), such as the PLOT command; however, some of the VDU commands, such as those that move the text cursor, are tied to OTF. Their will be more integration in the future. For the time being, in order to do any kind of graphics with OTF, use the VDU commands described in the OTF documentation.
* OTF supports multiple text areas, which means that you can have, for example, a full-screen text area plus one or more other text areas over it. Or you can have some other background (a solid rectangle, tile map, tile array, or large bitmap), and place one or more text areas over that.

[Home](otf_mode.md)
