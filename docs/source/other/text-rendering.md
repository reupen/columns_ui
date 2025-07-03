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

### Use greyscale anti-aliasing

Columns UI will use normally use ClearType anti-aliasing when ClearType is
enabled in Windows.

However, DirectWrite also includes a non-ClearType greyscale anti-aliasing mode,
which can be used in place of ClearType by enabling this option. This mode is
slightly faster than ClearType, and is preferred by Microsoft when using their
newer app technologies.

This option has no effect when anti-aliasing is disabled.

If ClearType is disabled in Windows but anti-aliasing is enabled, greyscale
anti-aliasing will be used automatically.

For LCD monitors, ClearType will normally have the best legibility.

Note that ClearType can also be put into a (different) greyscale mode using the
ClearType Text Tuner utility built into Windows. (This only affects text
rendered using DirectWrite or Windows Presentation Foundation.)

### Use colour glyphs when available

When enabled, colour emojis are used on Windows 8.1 and newer. Disable this
option to use monochrome emojis that use the current text colour.

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
