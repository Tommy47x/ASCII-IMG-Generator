# ASCII generator

Turn an image into ASCII art made of characters. This repo contains:

* small Python scripts that render an image as a grayscale or color ASCII image
* an ImGui desktop interface for picking files and adjusting the result

## Features

* Image to ASCII **image** output (`.jpg`, `.png`, ...), grayscale or color
* Black or white background
* Alphabet pools for English, German, French, Spanish, Italian, Portuguese, Polish, Russian, Chinese, Korean and Japanese
* Simple, complex and per-language character modes
* Adjustable output width in columns
* Color-output **Whiteness** control that lifts dark characters
* Desktop UI with drag-and-drop, presets, before/after preview and output actions

Sample images are in `demo/`.

## Desktop UI

Run:

```bash
./ui/run.sh
```

Or build manually:

```bash
cmake -S ui -B ui/build -DCMAKE_BUILD_TYPE=Release
cmake --build ui/build
./ui/build/ascii_studio
```

Build requirements: CMake 3.16+, a C++17 compiler, SDL2 and OpenGL development packages. Python 3 with Pillow is discovered by CMake and used for conversion.

The UI changes to the repository root at startup because the fonts and scripts are resolved relative to it. Conversion runs in a background thread; `Ctrl+Enter` is a shortcut for Generate.

## Command line

The UI calls these two scripts directly, so they can also be used on their own.

```bash
python3 img2img.py --input data/input.jpg --output data/output.jpg \
    --language english --mode standard --background black --num_cols 200

python3 img2img_color.py --input data/input.jpg --output data/output_color.jpg \
    --language japanese --mode katakana --background white --num_cols 150
```

Options:

| Flag | Meaning |
| --- | --- |
| `--input` | source image path |
| `--output` | destination image path |
| `--language` | alphabet pool to draw with |
| `--mode` | `simple` / `complex`, or the language-specific mode such as `hiragana` |
| `--background` | `black` or `white` |
| `--num_cols` | output width in characters |
| `--scale` | accepted by `img2img_color.py` for compatibility |

## Layout

| Path | Purpose |
| --- | --- |
| `img2img.py` | grayscale CLI entry point |
| `img2img_color.py` | color CLI entry point |
| `ascii_engine.py` | glyph ranking, grid mapping and rendering |
| `glyphsets.py` | character pools for each language/mode |
| `ui/` | ImGui desktop interface and its vendored libraries |
| `fonts/` | bundled fonts used by the renderer |
| `data/`, `demo/` | sample inputs and outputs |

## Requirements

* Python 3
* Pillow

The interface additionally needs SDL2, OpenGL and a C++17 compiler.

## Bundled components

The `ui/` folder includes Dear ImGui, its SDL2/OpenGL3 backends, stb_image and Inter; the `fonts/` folder includes DejaVu, SimSun and Arial Unicode. Their license files are kept alongside them.

## License

The scripts and the ImGui desktop interface in this repository are released under the [MIT License](LICENSE).

Bundled third-party components keep their own licenses:

* Dear ImGui, its SDL2/OpenGL3 backends and stb_image — `ui/third_party/imgui/LICENSE.txt`
* Inter — `ui/assets/Inter-LICENSE.txt`
* DejaVu — `fonts/DejaVu Fonts License.txt`
* SimSun and Arial Unicode are bundled font files; check their own terms before redistributing them.

The files under `demo/` are sample images included for reference and are not covered by the root license.
