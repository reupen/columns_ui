import argparse
import subprocess
from glob import iglob
from itertools import chain
from pathlib import Path, PurePath

OUTPUT_SIZES = [16, 20, 24, 32, 40, 48, 64, 256]

parser = argparse.ArgumentParser(
    description="Convert SVGs to ICOs. Requires Inkscape and ImageMagick to be on the path.",
)
parser.add_argument(
    'input_file',
    type=str,
    nargs='+',
    help='input SVG file',
)


def convert_file(input_file):
    name = PurePath(input_file).stem
    output_files = [(size, f"{name}-{size}.temp.png") for size in OUTPUT_SIZES]

    for size, output_file in output_files:
        commands = [
            "inkscape.com",
            "-w",
            f"{size}",
            "-o",
            output_file,
            input_file,
        ]
        result = subprocess.run(commands)
        result.check_returncode()

    output_file_names = [output_file[1] for output_file in output_files]
    commands = ["magick", "convert", *output_file_names, f"{name}.ico"]
    result = subprocess.run(commands)
    result.check_returncode()

    for _, output_file in output_files:
        Path(output_file).unlink()


def main():
    args = parser.parse_args()
    input_file_iters = [iglob(file, recursive=True) for file in args.input_file]
    for input_file in chain(*input_file_iters):
        convert_file(input_file)


if __name__ == '__main__':
    main()
