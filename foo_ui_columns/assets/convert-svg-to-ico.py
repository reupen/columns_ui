#!/usr/bin/env python3

import argparse
import subprocess
from itertools import chain
from pathlib import Path, PurePath

OUTPUT_SIZES = [16, 20, 24, 28, 32, 36, 40, 48, 56, 64, 256]
PARENT_DIR = Path(__file__).parent

parser = argparse.ArgumentParser(
    description="Convert SVGs to ICOs. Requires Inkscape and ImageMagick to be on the path.",
)

parser.add_argument(
    "input_file",
    type=str,
    nargs="+",
    help="input SVG file",
)

parser.add_argument(
    "output_dir",
    type=PurePath,
    help="output directory",
)

parser.add_argument(
    "--ignore",
    type=str,
    nargs="+",
    help="input filenames to ignore",
)


def convert_file(input_file, output_dir):
    name = input_file.stem
    output_files = [(size, f"{name}-{size}.temp.png") for size in OUTPUT_SIZES]

    for size, output_file in output_files:
        commands = [
            "inkscape",
            "-w",
            f"{size}",
            "-o",
            output_file,
            input_file,
        ]
        result = subprocess.run(commands)
        result.check_returncode()

    output_file_names = [output_file[1] for output_file in output_files]
    commands = ["magick", *output_file_names, output_dir / rf"{name}.ico"]
    result = subprocess.run(commands)
    result.check_returncode()

    for _, output_file in output_files:
        Path(output_file).unlink()


def main():
    args = parser.parse_args()
    input_file_iters = [PARENT_DIR.glob(file) for file in args.input_file]

    for input_file in chain(*input_file_iters):
        if input_file.name not in args.ignore:
            convert_file(input_file, args.output_dir)


if __name__ == "__main__":
    main()
