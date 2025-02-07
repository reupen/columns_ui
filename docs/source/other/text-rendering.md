# Text rendering

```{note}
This page applies to Columns UI 3.0.0 and newer.
```

Columns UI uses DirectWrite for text rendering in the following built-in panels
and parts of the UI:

- playlist view
- playlist switcher
- Filter panel
- Item details
- Item properties
- list views in Columns UI preferences
- status bar
- status pane

Other parts of the UI that are part of Columns UI itself continue to use
Uniscribe or GDI for text rendering.

Third-party panels will vary in their choice of text rendering technology.

## Text rendering options

Some DirectWrite-specific text rendering options are available on the Text
rendering tab of the Colours and fonts preferences page.

These options may not be supported by third-party panels.

### Force greyscale anti-aliasing

ClearType anti-aliasing will normally be used by default. If your monitor does
not have an RGB or BGR pixel layout, or has a high pixel density, you may want
to force greyscale anti-aliasing.

### Use colour glyphs when available

When enabled, colour emoji is used on Windows 8.1 and newer. Disable this option
to use monochrome emoji that uses the current text colour.

### Use alternative emoji font selection logic

When this option is enabled, the default DirectWrite emoji font selection logic
is replaced. The replacement logic:

- handles
  [emoji variation selectors](https://en.wikipedia.org/wiki/Emoji#Emoji_versus_text_presentation)
  for emojis that have both text (monochrome) and emoji presentation modes
- uses the default presentation mode of emojis as defined in the Unicode
  specification
- allows custom colour and monochrome emoji fonts to be used

[Noto Emoji](https://fonts.google.com/noto/specimen/Noto+Emoji) is an example of
an alternative monochrome emoji font that can be used with this option.

Note that Noto Color Emoji is not currently recommended, as it uses COLRv1 and
SVG glyphs that have limited support in Columns UI.
