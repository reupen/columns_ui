# Artwork image formats

The artwork view and playlist view use the Windows Imaging Component (WIC) to
load images.

This means that support for additional image formats can be added by installing
additional WIC codecs.

```{note}
When using newer or less common image formats, you may need to specify the
file extension explicitly when configuring artwork sources in Display
preferences (rather than use `.*`).
```

## WebP

Windows 10 and 11 normally has a WebP codec installed automatically.

It can also be installed manually from Microsoft Store:

- [WebP Image Extension](https://apps.microsoft.com/detail/9pg2dk419drg)

On older versions of Windows and on Wine,
[the Google WebP codec](https://storage.googleapis.com/downloads.webmproject.org/releases/webp/WebpCodecSetup.exe)
can be installed for WebP support. (Note that this was last updated in January
2016 and is no longer being maintained.)

## AVIF

For AVIF support on Windows 10 and 11 both of the following codecs must be
installed from Microsoft Store:

- [HEIF Image Extension](https://apps.microsoft.com/detail/9pmmsr1cgpwg)
- [AV1 Video Extension](https://apps.microsoft.com/detail/9mvzqvxjbq9v)

These may have been installed automatically.

## JPEG XL

For JPEG XL support, you can use either
[jxl-winthumb](https://github.com/saschanaz/jxl-winthumb/), or the
[JPEG XL Image Extension](https://apps.microsoft.com/detail/9mzprth5c0tb)
available from Microsoft Store.
