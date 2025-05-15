# SVG assets

The source SVGs for icons are in the `vectors` subdirectory.

After editing them, the minified SVGs and icon files need to be regenerated.

To do this,
[Docker Desktop](https://docs.docker.com/desktop/setup/install/windows-install/)
is required.

Once Docker Desktop installed and running, run `process-assets.cmd` to
regenerate the minified SVGs and rendered icon files.

Currently, there is no automated command to regenerate
`vectors\rendered\dark-placeholder-artwork.png` and
`vectors\rendered\light-placeholder-artwork.png`.
