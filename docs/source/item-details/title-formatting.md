# Title formatting (Item details)

## Functions

### \$set_font

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

Restores the font for subsequent text to the default panel font.

#### Syntax

```
$reset_font()
```
