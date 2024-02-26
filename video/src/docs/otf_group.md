<i>This is an early release of the OTF capability. Please read [Issues and Considerations](otf_issues.md)!</i>

## Create primitive: Group
<b>VDU 23, 30, 140, id; pid; flags; x; y; w; h;</b> : Create primitive: Group

This commmand creates a primitive that groups its child primitives,
for the purposes of motion and clipping. If a group node has
children, and the group node is moved, and the children are positioned
relative to their parent (i.e., they do not use the PRIM_FLAG_ABSOLUTE flag),
then the children are moved with the parent. Note that a group node
has no visible representation (i.e., is not drawn).

When this command is used, OTF will automatically clear the PRIM_FLAG_PAINT_THIS flag.

Changing the flags of a group node can show or hide its children.

[Home](otf_mode.md)
