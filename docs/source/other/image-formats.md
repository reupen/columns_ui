# Artwork image formats

The artwork view and playlist view use the Windows Imaging Component (WIC) to
load images.

This means that support for additional image formats can be added by installing
additional WIC codecs.

The current version of Windows 10 and 11 normally have WebP and HEIF images
installed automatically. These can also be manually installed from the Microsoft
Store if needed:

- [WebP image extensions](https://www.microsoft.com/en-gb/p/webp-image-extensions/9pg2dk419drg)
- [HEIF image extensions](https://www.microsoft.com/en-gb/p/heif-image-extensions/9pmmsr1cgpwg)

On older versions of Windows and on Wine,
[the Google WebP codec](https://storage.googleapis.com/downloads.webmproject.org/releases/webp/WebpCodecSetup.exe)
can be installed for WebP support.

For JPEG XL support, you can install
[jxl-winthumb](https://github.com/saschanaz/jxl-winthumb/).

```{note}
When using newer or less common image formats, you may need to specify the
file extension explicitly when configuring artwork sources in Display
preferences (rather than use `.*`).
```
