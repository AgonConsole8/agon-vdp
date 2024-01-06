<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

# OTF Critical Section

There are certain aspects to how the OTF mode draws (paints) the screen that
should be kept in mind when designing the graphics for applications.

* <b>Timing is everything.</b> At a pixel clock rate of 40 MHz, each scan line,
including the visible and invisible portions of it, lasts just 0.0000264 seconds
(which is 26.4 microseconds). Not a lot of time! At the 240 MHz internal clock
rate of the ESP32, each clock tick lasts about 0.004167 microseconds (which is
4.167 picoseconds). Dividing the scan line time by the CPU clock tick time
indicates that there are approximately 6336 CPU clock cycles per scan line.
During those cycles, the CPU must execute enough instructions to draw one line.
While one scan line is being displayed by the I2S hardware, the OTF manager must
draw (create 800 colored pixels for) a future scan line, and it must do so in under
6336 CPU clock cycles. Again, not a lot of time.<br><br>For this reason, it is critical to avoid drawing (layering) too many things on the same scan line. Each time the same pixel in the current scan line buffer is written,
time is used, so if there are too many objects (primitives) layered on top of
each other, there may not be enough time to draw the scan line, before the next
scan line should be drawn. If time runs out, the screen will show flashing
(flickering) scan lines. It is unlikely that an entire object (such as a sprite)
will flicker on and off. What is much more likely is that distinct scan lines
within the object will flicker, or be vertically out of place.<br><br>Another reason that flickering may occur, meaning that the time to draw a scan line exceeds the limit, is that CPU time can be stolen away from the OTF manager
via interrupts and by other tasks. To help to reduce this possibility, the OTF manager runs in a high-priority task.

* <b>Everything must be painted.</b> Every scan line on the screen (all 600 of them) must be drawn (painted) on every frame (i.e., 60 times per second). What
happens if one of those scan lines is not painted? Whatever was left in the scan
line buffer will be displayed again, at whatever vertical position the I2S hardware happens to be outputting. To exaggerate a bit, if the code only painted the first (top) scan line (out of the 8 lines used) once, and never again, then the display would show that scan line's pixels on the screen 75 times, once per group of 8 lines, going down the screen's 600 lines.<br><br>
So, how does this requirement affect the EZ80 application, and how does it affect the OTF manager in the ESP32? Let's take the latter part first. In order
to redraw each active primitive 60 times per second, the manager must keep them
all in RAM (as mentioned above). Primitives are not transferred, written, and
deleted. They are stored for continual use, until the EZ80 application sends the
OTF commands to delete them. There is a big advantage to having them kept in RAM, because they can be used for a long time, without being transferred across
the serial port again (i.e., sending the command(s) to create a primitive once
is enough).<br><br>
Now, on to the former part. In order to make sure that every scan line is painted during each frame, the EZ80 application must define at least 1 primitive
that covers each pixel on the screen, so that there is at least one "drop" of
paint landing on that pixel during frame output. If any pixel is missed, it will
not be painted, and will be shown multiple times with the last drop of paint
that it received (most likely from somewhere else on the screen).<br><br>
There are 2 simple ways to be sure that every pixel is painted by at least 1
layer of paint. The first alternative is to create a text area primitive, or a custom
tile array, that covers the entire screen. The text area, if full-screen, implies a tile array of characters in 8x8-pixel cells, totalling 100
columns across by 75 rows down. Every pixel on the screen is covered by such a
text area primitive (at present there is no way to make it smaller; that is
a future enhancement). The second alternative is to create a solid rectangle primitive with the
X/Y coordinates at (0, 0), a width of 800 pixels, and a height of 600 pixels.
Such a rectangle can be any of the 64 colors, and would serve as a background
for the entire video frame. Other things could be drawn on top of it.<br><br>
Of course, you have the option of piecing together various primitives of
your choosing, as long as you cover the entire screen.<br><br>
To be clear, if the EZ80 application does not define some kind of background
(often, either a full tile array or a solid rectangle), then some part(s) of the
screen will not be painted properly, giving undesirable effects.

[Home](otf_mode.md)
