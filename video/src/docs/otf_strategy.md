<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

# OTF Strategy

In the OTF mode, each drawing primitive is a node in a tree of primitives. The tree branches (nesting) can
serve to affect how primitives are moved, drawn, and clipped. For example, in the simplest case, if primitve B
is a child of primitive A, and A is moved (in X and/or Y), then B is moved along with A, so that its relative
postion with respect to A does not change, but its physical position on the display does change. Thus, an entire array of enemy ships can
be moved simply by moving a single "group" primitive that is the parent of all of the ships.

Another important aspect of how the OTF mode works is that it must keep all primitives in internal RAM,
so that it can draw them repeatedly, without being told to do so by the BASIC (EZ80) application. Since
the ESP32 holds the primitives in RAM, they can easily be manipulated (e.g., moved), without sending
all of the information required to recreate them across the serial channel. VDP-GL already does something
similar with bitmaps and sprites, but as a Fab-GL derivative, not with simpler primitives such as lines and triangles. However,
enhancements are in the works that allow VDP-GL to create buffered
commands, so that it can replay them without a large data transfer.

When primitives are no longer needed by the BASIC application, they may be deleted. Deleting a group
primitive will delete its children, too, reducing the number of serial commands required to delete
the parent and all of its children.

One other benefit of keeping the primitives in RAM is the ability to turn them on and off, meaning
to show them or to hide them, very easily, just by changing their flag bits. For example, a blinking
cursor could be actuated just by changing its flag bit for being painted (drawn). The cursor itself can
be any primitive (a point, line, rectangle, etc.), based on the needs of the application.

[Home](otf_mode.md)
