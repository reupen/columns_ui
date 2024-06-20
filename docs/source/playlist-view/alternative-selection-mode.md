# Alternative selection mode

The alternative selection mode partially emulates selection behaviour from early
versions of foobar2000.

In this mode, the behaviour when Shift while pressing Up, Down, PgUp, PgDn, Home
or End is modified. If the focused item was selected, this action will move the
focus while expanding the selection (without deselecting any tracks).

If the focused item was not selected, the focus will move and any items between
the old and new focused item will be deselected.

This modified behaviour can be combined with Ctrl+Space to build non-contiguous
selections using the keyboard (albeit in a clunky manner).
