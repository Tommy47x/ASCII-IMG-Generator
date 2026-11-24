"""Image-to-ASCII-image rendering used by the two CLI entry points.

Everything here is implemented with PIL: it measures how much ink each glyph
lays down, builds a dark-to-light ramp, then paints one glyph per image cell.
"""
from functools import lru_cache
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont, ImageOps, ImageStat

from glyphsets import glyph_pool

ROOT = Path(__file__).resolve().parent
FONTS = ROOT / "fonts"

# font file, point size, vertical stretch, and a representative glyph
FONT_SPECS = {
    "general": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "english": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "german": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "french": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "spanish": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "italian": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "portuguese": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "polish": ("DejaVuSansMono-Bold.ttf", 20, 2, "A"),
    "russian": ("DejaVuSansMono-Bold.ttf", 20, 2, "Ш"),
    "chinese": ("simsun.ttc", 10, 1, "制"),
    "korean": ("arial-unicode.ttf", 10, 1, "ㅊ"),
    "japanese": ("arial-unicode.ttf", 10, 1, "あ"),
}


@lru_cache(maxsize=32)
def load_font(language, size_override=None):
    filename, size, stretch, sample = FONT_SPECS.get(language, FONT_SPECS["english"])
    path = FONTS / filename
    font = ImageFont.truetype(str(path), size if size_override is None else size_override)
    box = font.getbbox(sample)
    return font, max(1, box[2] - box[0]), max(1, box[3] - box[1]), stretch


@lru_cache(maxsize=64)
def density_ramp(language, mode):
    """Sort glyphs by measured ink coverage, densest first."""
    font, _, _, _ = load_font(language)
    scored = []
    for glyph in glyph_pool(language, mode):
        box = font.getbbox(glyph)
        width = max(1, box[2] - box[0])
        height = max(1, box[3] - box[1])
        tile = Image.new("L", (width, height), 255)
        ImageDraw.Draw(tile).text((-box[0], -box[1]), glyph, fill=0, font=font)
        ink = 255 - sum(tile.getdata()) / (width * height)
        scored.append((ink, glyph))
    scored.sort(key=lambda pair: pair[0], reverse=True)
    return tuple(glyph for _, glyph in scored)


def character_ramp(language, mode):
    ordered = density_ramp(language, mode)
    if language != "general":
        return ordered
    count = 10 if mode == "simple" else 70
    if len(ordered) <= count:
        return ordered
    last = len(ordered) - 1
    return tuple(ordered[round(i * last / (count - 1))] for i in range(count))


def render_image(input_path, output_path, language, mode, background, num_cols, color):
    """Render one ASCII image file. `color=True` keeps each cell's mean RGB."""
    language = language.lower()
    font, char_w, char_h, stretch = load_font(language)
    ramp = character_ramp(language, mode)

    with Image.open(input_path) as opened:
        source = opened.convert("RGB" if color else "L")
    width, height = source.size

    columns = max(1, min(int(num_cols), width))
    cell_w = width / columns
    cell_h = stretch * cell_w
    rows = max(1, int(height / cell_h))

    bg_value = 255 if background == "white" else 0
    canvas_mode = "RGB" if color else "L"
    bg_fill = (bg_value, bg_value, bg_value) if color else bg_value
    canvas = Image.new(canvas_mode, (char_w * columns, char_h * rows), bg_fill)
    pen = ImageDraw.Draw(canvas)

    for row in range(rows):
        top = int(row * cell_h)
        bottom = min(height, max(top + 1, int((row + 1) * cell_h)))
        for col in range(columns):
            left = int(col * cell_w)
            right = min(width, max(left + 1, int((col + 1) * cell_w)))
            cell = source.crop((left, top, right, bottom))

            means = ImageStat.Stat(cell).mean
            brightness = sum(means) / len(means)
            index = min(len(ramp) - 1, int(brightness * len(ramp) / 256))
            glyph = ramp[index]

            if color:
                fill = tuple(int(value) for value in means)
            else:
                fill = 255 - bg_value
            pen.text((col * char_w, row * char_h), glyph, font=font, fill=fill)

    box = ImageOps.invert(canvas).getbbox() if background == "white" else canvas.getbbox()
    if box:
        canvas = canvas.crop(box)
    output = Path(output_path)
    if output.parent != Path(""):
        output.parent.mkdir(parents=True, exist_ok=True)
    canvas.save(output_path)
