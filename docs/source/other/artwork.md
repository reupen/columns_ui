# Artwork

## Image formats

The artwork view and playlist view use the Windows Imaging Component (WIC) to
load images.

This means that support for additional image formats can be added by installing
additional WIC codecs.

```{note}
When using newer or less common image formats, you may need to specify the
file extension explicitly when configuring artwork sources in Display
preferences (instead of using `.*`).
```

### WebP

Windows 10 and 11 normally has a WebP codec installed automatically.

It can also be installed manually from Microsoft Store:

- [WebP Image Extension](https://apps.microsoft.com/detail/9pg2dk419drg)

On older versions of Windows and on Wine,
[the Google WebP codec](https://storage.googleapis.com/downloads.webmproject.org/releases/webp/WebpCodecSetup.exe)
can be installed for WebP support. (Note that this was last updated in January
2016 and is no longer being maintained.)

### AVIF

For AVIF support on Windows 10 and 11 both of the following codecs must be
installed from Microsoft Store:

- [HEIF Image Extension](https://apps.microsoft.com/detail/9pmmsr1cgpwg)
- [AV1 Video Extension](https://apps.microsoft.com/detail/9mvzqvxjbq9v)

These may have been installed automatically.

### JPEG XL

For JPEG XL support, you can use either
[jxl-winthumb](https://github.com/saschanaz/jxl-winthumb/), or the
[JPEG XL Image Extension](https://apps.microsoft.com/detail/9mzprth5c0tb)
available from Microsoft Store.

## Colour management

```{note}
New in version 3.1.0.
```

Both the Artwork view and playlist view support colour management for images,
including using image colour profiles and targeting the colour space of your
display.

If HDR is enabled in Windows, you can also enable Advanced Colour support in the
Artwork view to improve support for HDR and high-bit-depth images. It’s not
recommended to enable Advanced Colour if HDR is turned off, as that’s known to
produce incorrect results.

If HDR or
[‘Automatically manage colour for apps’](https://support.microsoft.com/en-gb/windows/change-display-brightness-and-color-in-windows-3f67a2f2-5c65-ceca-778b-5858fc007041#bkmk_color_profile)
are enabled in Windows, the playlist view and the Artwork view (with Advanced
Colour disabled) will be limited to the sRGB colour space by default. To restore
wide-colour-gamut support, enable
[‘Use legacy display ICC colour management’](https://learn.microsoft.com/en-gb/windows/win32/wcs/advanced-color-icc-profiles#display-icc-profile-compatibility-helper)
in the properties for foobar2000.exe (or a shortcut to foobar2000.exe).

Note that other parts of Columns UI are not colour-managed, and other components
will have their own approach to colour management.
