<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

# OTF Dynamic Code Generation

As mentioned above, the OTF mode uses some built-in ESP32 assembler functions to help with drawing primitives quickly; however, it
goes a step further. In order to reduce the number of decisions made
while drawing, it uses the primitive definitions to generate portions
of ESP32 code at runtime. In some cases, instructions are generated
that call built-in functions in whatever sequence is required.
In other cases, instructions are generated that write to the specific
pixel data bytes.
<br><br>
For example, to draw a 20 pixel long line from X position 39 to X position 58, inclusive, the OTF mode will generate instructions to
set the single pixel at 39, set the 16 pixels from 40 to 55 (4 pixels at a time), set 2 pixels at 56 to 57, and finally, set the remaining
single pixel at 58.
<br><br>
The code generation operation itself does take a small amount of time,
and depending on how many primitives are processed to do so, there
may be some temporary effect on painting the screen, meaning
that it may flicker or duplicate scan lines during that time.

[Home](otf_mode.md)
