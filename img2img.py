"""Convert an image into a grayscale ASCII image."""
import argparse

from ascii_engine import render_image


def parse_args():
    parser = argparse.ArgumentParser(description="Image to grayscale ASCII image")
    parser.add_argument("--input", default="data/input.jpg")
    parser.add_argument("--output", default="data/output.jpg")
    parser.add_argument("--language", default="english")
    parser.add_argument("--mode", default="standard")
    parser.add_argument("--background", choices=["black", "white"], default="black")
    parser.add_argument("--num_cols", type=int, default=300)
    return parser.parse_args()


def main():
    args = parse_args()
    render_image(
        input_path=args.input,
        output_path=args.output,
        language=args.language,
        mode=args.mode,
        background=args.background,
        num_cols=args.num_cols,
        color=False,
    )


if __name__ == "__main__":
    main()
