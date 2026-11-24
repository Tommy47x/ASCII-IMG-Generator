"""Lift dark pixels in a generated color ASCII image.

Called by the ImGui UI after img2img_color.py. A gamma-style lookup table
brightens midtones and dark colours while leaving pure black (the background)
and pure white untouched.
"""
import argparse
import os
import tempfile

from PIL import Image


def get_args():
    parser = argparse.ArgumentParser("Whiten ASCII output")
    parser.add_argument("--input", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--whiteness", type=int, default=0,
                        help="0 keeps the image unchanged, 100 is the brightest")
    return parser.parse_args()


def main(opt):
    amount = max(0, min(100, opt.whiteness))
    if amount == 0:
        if os.path.abspath(opt.input) != os.path.abspath(opt.output):
            with Image.open(opt.input) as img:
                img.save(opt.output)
        return

    gamma = 1.0 + (amount / 100.0) * 1.8
    lut = [min(255, int(round(255 * ((i / 255.0) ** (1.0 / gamma)))))
           for i in range(256)]

    with Image.open(opt.input) as img:
        image = img.convert("RGB")
        adjusted = image.point(lut * 3)

        directory = os.path.dirname(os.path.abspath(opt.output)) or "."
        suffix = os.path.splitext(opt.output)[1] or ".png"
        fd, tmp_path = tempfile.mkstemp(prefix=".ascii_whiten_", suffix=suffix,
                                        dir=directory)
        os.close(fd)
        try:
            if suffix.lower() in (".jpg", ".jpeg"):
                adjusted.save(tmp_path, quality=95)
            else:
                adjusted.save(tmp_path)
            os.replace(tmp_path, opt.output)
        finally:
            if os.path.exists(tmp_path):
                os.remove(tmp_path)


if __name__ == "__main__":
    main(get_args())
