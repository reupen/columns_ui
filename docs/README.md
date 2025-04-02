# Columns UI documentation site

This directory contains a documentation site built using
[Sphinx](https://www.sphinx-doc.org/).

The site is published at https://columns-ui.readthedocs.io.

## Build instructions

1. Install Python 3.12.

2. Install [uv](https://docs.astral.sh/uv/getting-started/installation/).

3. Switch to this directory.

4. Run:

   ```shell
   uv sync
   ```

5. Run:

   ```shell
   uv run .\make.bat clean
   ```

6. Run:

   ```shell
   uv run .\make.bat html
   ```

The built docs will then be in the `build\html` directory.
