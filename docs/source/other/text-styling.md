# Text styling

Built-in panels support changing text styling for parts of text via
`$set_format()`, `$rgb()` and related title formatting functions. This page
provides documentation for those functions and some related fields.

## Functions

### \$set_format()

```{note}
Available in the following contexts from Columns UI 3.0.0:

- Item details

Available in the following contexts from Columns UI 3.3.0:

- Filter panel
- Playlist switcher
- Playlist view
- Status bar
- Status pane
```

Changes font and text styling for subsequent text.

#### Syntax

```
$set_format(
  property-name-1: property-value-1;
  property-name-2: property-value-2;
  ...
)
```

The final semicolon is optional.

#### Properties

| Property name     | Syntax                                         |
| ----------------- | ---------------------------------------------- |
| `font-family`     | \<font family name> \| `initial`               |
| `font-size`       | \<font size in points> \| `initial`            |
| `font-weight`     | \<1–999> \| `normal` \| `bold` \| `initial`    |
| `font-stretch`    | \<1–9> \| \<percentage> \| `initial`           |
| `font-style`      | `normal` \| `italic` \| `oblique` \| `initial` |
| `text-decoration` | `none` \| `underline` \| `initial`             |

```{note}
Except in Item details, increasing the the font size does not increase the
line or item height. Increasing the font size will cause the text baseline
to move downwards, and may not yield the desired result in locations such
as playlist view cells.
```

The special `initial` value resets any particular property back to its default
value.

Percentages must use the suffix `%%` or `pc`, for example `150%%` or `150pc`
(`%%` is an escaped `%` in the title formatting language).

#### Usage in playlist view style scripts

When used in a playlist view style script, `$set_format()` has the same effect
as being used at the beginning of a display script.

It does not affect the behaviour of the `initial` special value or
`$reset_format()`.

#### Examples

##### Set all properties

```
$set_format(
  font-family: Segoe UI Variable;
  font-size: 20;
  font-weight: 300;
  font-stretch: 100%%;
  font-style: italic;
  text-decoration: underline;
)
```

##### Change the font weight temporarily

```
$set_format(
  font-weight: 700;
)

This text is in bold.

$set_format(
  font-weight: initial;
)
```

##### Use icons from Symbols Nerd Font

Symbols Nerd Font must be
[downloaded and installed](https://www.nerdfonts.com/font-downloads).

```
$set_format(font-family: Symbols Nerd Font)
󱑽
$reset_format()
```

### \$reset_format()

```{note}
Available in the following contexts from Columns UI 3.0.0:

- Item details

Available in the following contexts from Columns UI 3.3.0:

- Filter panel
- Playlist switcher
- Playlist view (display, grouping and global variables scripts)
- Status bar
- Status pane
```

Restores font and text styling for subsequent text to the panel defaults.

#### Syntax

```
$reset_format()
```

### \$rgb()

```{note}
This function is provided by the foobar2000 title formatting engine. Custom
text colours cannot be used in the main window title.
```

Sets the text colour using red, green and blue component values.

#### Syntax

```
$rgb(r1,g1,b1)
$rgb(r1,g1,b1,r2,b2,g2)
```

`r1`, `g1` and `b1` are values between 0 and 255 that specify the red, green and
blue components of unselected text.

`r2`, `g2` and `b2` specify the colour of selected text. If `r2`, `g2` and `b2`
are not provided, then the inverse of the colour specified by `r1`, `g1` and
`b1` is used for selected text.

### \$hsl()

```{note}
This function is provided by the foobar2000 title formatting engine. Custom
text colours cannot be used in the main window title.
```

Sets the text colour using hue, saturation and luminance component values.

#### Syntax

```
$hsl(h1,s1,l1)
$hsl(h1,s1,l1,h2,s2,l2)
```

`h1` is a value between 0 and 239 that specifies the hue component of unselected
text.

`s1` and `l1` are values between 0 and 240 that specify the saturation and
luminance components of unselected text.

`h2`, `s2` and `l2` specify the colour of selected text. If `h2`, `s2` and `l2`
are not provided, then the inverse of the colour specified by `h1`, `s1` and
`l1` is used for selected text.

### \$blend()

```{note}
This function is provided by the foobar2000 title formatting engine. Custom
text colours cannot be used in the main window title.
```

Returns a colour that is part way between two other colours.

#### Syntax

```
$blend(colour one,colour two,numerator,denominator)
```

The colour that is numerator/denominator the way between colour one and colour
two will be returns.

#### Example

This will colour the text of tracks from grey to white by track number:

```
$blend($rgb(128,128,128),$rgb(255,255,255),$sub(%tracknumber%,1),$sub(%totaltracks%,1))
```

### \$transition()

```{note}
This function is provided by the foobar2000 title formatting engine. Custom
text colours cannot be used in the main window title.
```

Applies a colour gradient to a string. Each letter in the string is a solid
colour.

#### Syntax

```
$transition(text,start colour,end colour)
```

#### Example

```
$transition(this string starts red and ends blue,$rgb(255,0,0),$rgb(0,0,255))
```

### \$set_font()

```{note}
Available in the following contexts:

- Item details
```

```{warning}
Deprecated in Columns UI 3.0.0. It’s been replaced by $set_format().
```

Changes the font used for subsequent text.

#### Syntax

```
$set_font(<font face>,<point size>,<modifiers>)
```

where modifiers are semicolon separated values from the below list:

- `bold`
- `italic`
- `underline`

You can store the output of `$set_font` using `$put` or `$puts` to enable you to
easily recall the font later using `$get`.

#### Examples

##### Setting the font to Segoe UI 12 pt, bold, italic (short form)

```
$set_font(Segoe UI,12,bold;italic;)
```

##### Setting the font to Segoe UI 12 pt, bold, italic (full form)

```
$set_font(Segoe UI,12,bold=true;italic=true;)
```

##### Storing fonts for repeated/later use

```
$puts(labelfont,$set_font(Segoe UI,12,bold;italic;))

$get(labelfont)Artist$reset_font() %artist%
$crlf()
$get(labelfont)Title$reset_font() %title%
```

### \$reset_font()

```{note}
Available in the following contexts:

- Item details
```

```{warning}
Deprecated in Columns UI 3.0.0. It’s been replaced by $reset_format().
```

Restores font and text styling for subsequent text to the panel defaults.

#### Syntax

```
$reset_font()
```

## Fields

### %default_font_size%

```{note}
Available in the following contexts:

- Item details

Available in the following contexts from Columns UI 3.3.0:

- Filter panel
- Playlist switcher
- Playlist view
- Status bar
- Status pane
```

The default font size for the panel as configured on the Colours and fonts
preferences page, in points. The value is rounded to the nearest whole number in
Columns UI 3.2.x and older, and to one decimal place in Columns UI 3.3.0 and
newer.

### %default_font_face%

```{note}
Available in the following contexts:

- Item details
```

```{warning}
Deprecated in Columns UI 3.0.0. Use ``$set_format(font-family: initial)`` instead.
```

The default font family for the panel. This is a legacy, GDI-compatible font
family name.

## Tab columns

```{note}
This is a legacy feature dating back to early versions of foobar2000.

It’s available in the following contexts:

- Filter panel
- Playlist switcher
- Playlist view
- Status bar
- Status pane
```

Tab characters can be used in the above contexts to align text across multiple
items, such that columns are formed. Text can also be right-aligned at the end
of the available space.

Consider the following example:

```
column 1
$tab()
column 2
$tab()
right-aligned text
```

(Note that `$tab()` inserts a tab character.)

In this example, column 1 and column 2 are both equal sizes (i.e. 50% of the
available space). The text after the final tab character is right-aligned. It
shares space with the other columns and will cover them if text overlaps.

Text colours and formatting properties set using `$rgb()`, `$set_format()` and
similar functions are only effective until the next tab character is
encountered.

The size of the columns can be controlled by the number of tab characters. In
this example, the size of the first column is increased by adding a second tab
character after the column:

```
⅔ width column
$tab(2)
⅓ width column
$tab()
right-aligned text
```

(Note that `$tab(2)` inserts two tab characters.)

In this example, the size of the second column is increased:

```
⅓ width column
$tab()
⅔ width column
$tab(2)
right-aligned text
```

In more general terms, the proportion of space a column is given is the number
of consecutive tab characters following the column divided by the total number
of tab characters in the formatted string.

If right-aligned text is not required, use a trailing `$tab()` without any
following text:

```
column 1$tab()column 2$tab()
```
