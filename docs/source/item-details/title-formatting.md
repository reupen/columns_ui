# Title formatting (Item details)

## Functions

### \$set_format

```{note}
New in Columns UI 3.0.0.
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

#### Properties

| Property name     | Syntax                                         |
| ----------------- | ---------------------------------------------- |
| `font-family`     | \<font family name> \| `initial`               |
| `font-size`       | \<font size in points> \| `initial`            |
| `font-weight`     | \<1–999> \| `initial`                          |
| `font-stretch`    | \<1–9> \| \<percentage> \| `initial`           |
| `font-style`      | `normal` \| `italic` \| `oblique` \| `initial` |
| `text-decoration` | `none` \| `underline` \| `initial`             |

The special `initial` value resets any particular property back to its default
value.

Percentages must use the suffix `%%` or `pc`, for example `150%%` or `150pc`
(`%%` is an escaped `%` in the title formatting language).

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

### \$reset_format

```{note}
New in Columns UI 3.0.0.
```

Restores font and text styling for subsequent text to the panel defaults.

#### Syntax

```
$reset_format()
```

### \$set_font

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

### \$reset_font

```{warning}
Deprecated in Columns UI 3.0.0. It’s been replaced by $reset_format().
```

Restores font and text styling for subsequent text to the panel defaults.

#### Syntax

```
$reset_font()
```
