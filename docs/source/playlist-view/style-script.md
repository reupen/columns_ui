# Style script

The style script is a title formatting script that determines the colour of a
cell in the playlist. The global style script sets the default for all columns.
Per-column style scripts can be used to set colours for a particular column.

## Functions

### \$set_style()

A single function, `$set_style()`, is used to set colours. Colours are specified
using the `$rgb()` function.

```{note}
Selection colours cannot be overridden if the chosen colour scheme
for the playlist view is Themed on the Colours and fonts preferences page. You
must use the Custom or System scheme to override selection colours.
```

#### Syntax

##### Setting the text colour

You can set the text colour as follows:

```
$set_style(text,<text colour>,<selected item text colour>[,<selected item text colour when window is not focused>])
```

##### Setting the background colour

You can set the background colour as follows:

```
$set_style(back,<background colour>,<selected item background colour>[,<selected item background colour when window is not focused>])
```

##### Setting the group line style

Horizontal lines to the right of group headings can be configured using the
global style script.

To hide these lines, use:

```
$set_style(group-line,false)
```

To change the colour of these lines, use:

```
$set_style(group-line,true,<colour>)
```

where `<colour>` is a colour code.

For example, this makes these lines red:

```
$set_style(group-line,true,$rgb(255,0,0))
```

##### Setting the frame style

You can set the frame style as follows:

```
$set_style(<frame part>,<enabled state>[,<colour>])
```

where

- `<frame part>` is either `frame-top`,`frame-left`,`frame-bottom`,`frame-right`
- `<enabled state>` is any of `true`, `1`, `false` or `0`
- `<colour>` is the colour of the frame, required if `<enabled state>` is `1`.

```{note}
Support for using `true` and `false` for the second paramter, instead of `0` and `1`, was added in Columns UI 3.5.0
```

#### Example

```
$set_style(text,$rgb(255,0,0),$rgb(0,0,0))
$set_style(back,$rgb(255,255,255),$rgb(255,255,0))
$set_style(frame-left,1,$rgb(0,0,0))
```

This example will set colours as follows:

- Text – red
- Selection text – black
- Background – white
- Selection background – yellow
- Cell frame on left side only – black

### \$calculate_blend_target()

Returns black if the mean of the red, green and blue components of a colour is
greater than or equal to 128; otherwise it returns white.

#### Syntax

```
$calculate_blend_target(<colour>)
```

### \$offset_colour()

Shifts one colour towards another colour.

#### Syntax

```
$offset_colour(<colour_from>,<colour_to>,<offset>)
```

Intended to be used with black or white for `<colour_to>`.

`<offset>` should be between 0 and 255.

## Fields

### Colours

These fields return the relevant colour before any modifications by the current
style script.

| Field                       |
| --------------------------- |
| `%_text%`                   |
| `%_selected_text%`          |
| `%_selected_text_no_focus%` |
| `%_back%`                   |
| `%_selected_back%`          |
| `%_selected_back_no_focus%` |

### Other

| Field              | Description                                                                                             |
| ------------------ | ------------------------------------------------------------------------------------------------------- |
| `%_display_index%` | The current row number in the playlist view (including group headings)                                  |
| `%_is_group%`      | Whether a group heading is currently being formatted                                                    |
| `%_is_themed%`     | Whether ‘Themed’ has been selected as the scheme for the playlist view in Colours and fonts preferences |
