# FountEngine SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B23)

**FountEngine SDK** is the content pipeline for [FountEngine](https://github.com/ImHartash/FountEngine).
It's a set of small, single-purpose command-line tools that convert human-editable source assets
(images, `.obj` models, plain-text material descriptions) into the compact binary formats the engine
actually loads at runtime.

The split mirrors a common pattern in commercial engines (Source 2's content/game-side compiled
resources, for instance): you never hand-edit or ship the binary formats directly — you edit the
source, and the SDK compiles it.

Alongside the command-line converters, this repository also includes a Qt-based GUI asset browser
(`SdkTool`) — a project tree with drag-and-drop auto-conversion, a material creation form, and a
`.fntpk` content viewer, built on top of the same four command-line tools rather than replacing
them.

---

## Tools

| Tool | Input | Output |
|---|---|---|
| `fntmdl` | Wavefront `.obj` | `.fntmdl` — mesh geometry |
| `fnttex` | `.png`, `.jpg` | `.fnttex` — uncompressed GPU texture |
| `fntmat` | Plain-text material description (`.txt`) | `.fntmat` — compiled material |
| `fntpk` | A directory of compiled assets | `.fntpk` — package archive |

Executable names above match the file extension they produce, minus the leading `fnt` being shared
across all four — `fntmdl` builds `.fntmdl`, and so on.

---

## Formats

### `.fntmdl` — Model

`FNTMDL_HEADER` (magic, version, vertex/index counts, offsets, flags) followed by a vertex array
(position, normal, UV) and a `uint32_t` index array. Only static geometry — no skinning, no
animation. Only Wavefront `.obj` is accepted as input; `.fbx`/`.gltf`/etc. are not supported.

### `.fnttex` — Texture

`FNTTEX_HEADER` (magic, version, width, height, mip levels, `DXGI_FORMAT`) followed by raw pixel
data for each mip level, largest first. Textures are stored **uncompressed**
(`DXGI_FORMAT_R8G8B8A8_UNORM`) — no block compression yet.

### `.fntmat` — Material

`FNTMAT_HEADER` + a data block containing: diffuse texture path, ambient/diffuse/specular color,
shininess, opacity, and three enums — blend mode (`Opaque` / `AlphaBlend` / `Additive`), cull mode
(`Back` / `Front` / `None`), and depth mode (`Enabled` / `Disabled` / `ReadOnly`). Compiled from a
simple key-value text descriptor, e.g.:

```
$DiffuseTexture textures/test_cube.fnttex
$Ambient 0.2 0.2 0.2
$Diffuse 0.8 0.8 0.8
$Specular 0.5 0.5 0.5
$Shininess 32
$Opacity 1.0
$BlendMode Opaque
$CullMode Back
$DepthMode Enabled
```

### `.fntpk` — Package

`FNTPK_HEADER` (magic, version, flags, CRC, block count) followed by an array of `FNTPK_BLOCK`
entries (256-byte name, data offset, data size), then the raw concatenated file data. Uncompressed,
for fast seek-based loading — not a general-purpose archive format.

---

## Usage

```bash
fntmdl <input.obj> <output.fntmdl>
fnttex <input_image> [output.fnttex] [max_mip_maps]
fntmat <input_descriptor.txt> [output.fntmat]
fntpk  <input_directory> <output.fntpk>
```

`fnttex` and `fntmat` will infer the output path from the input filename if omitted.

`fntpk` walks the input directory recursively and packs every file it finds, storing paths relative
to the directory root as resource names.

---

## Typical Workflow

1. Author source assets: `character.png`, `level.obj`, a `.fntmat` text descriptor per material.
2. Run each source file through the matching converter.
3. Pack the resulting `.fntmdl` / `.fnttex` / `.fntmat` files into a `.fntpk` with `fntpk`.
4. Point FountEngine at the package; it loads assets by resource name.

---

## SdkTool (GUI)

A Qt Widgets application that wraps the four command-line converters in a project/asset browser:

- Project tree (filtered to recognized asset extensions) with drag-and-drop — dropping a source
  file (`.obj`, `.png`/`.jpg`) into a folder converts it automatically, using whichever converter
  matches the extension.
- A form-based material creator — fills in a `.fntmat` descriptor and bakes it, without hand-writing
  the text file.
- A `.fntpk` content viewer/editor — inspect, add, and remove entries in an existing package without
  rebuilding it from a source directory.
- Configurable paths (converters folder, project folder, pack output) persisted between runs.

Converter executable paths, project folder, and pack output settings are configured once via
**Settings** on first run.

---

## Building from Source

### Command-line converters + SdkTool — via CMake (cross-platform)

The whole SDK, including `SdkTool`, builds through a single top-level CMake project.

**Prerequisites:**
- CMake 3.16+
- A C++20 compiler (MSVC, GCC, or Clang)
- Qt 6 (`Widgets` component) — only required for `SdkTool`; the four command-line converters have no
  Qt dependency and build without it

**Steps:**
```bash
git clone https://github.com/ImHartash/FountEngine_SDK.git
cd FountEngine_SDK
mkdir build && cd build
cmake ..
cmake --build .
```

All resulting binaries (`fntmdl`, `fnttex`, `fntmat`, `fntpk`, `SdkTool`) are placed under `bin/`
inside the build directory.

If CMake can't locate Qt automatically, point it at your Qt install explicitly:
```bash
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" ..
```

### Command-line converters only — via Visual Studio

The original `.sln`/`.vcxproj` files are still included for building just the four converters
without Qt or CMake:

1. Open `FountEngine_SDK.sln` in Visual Studio 2022.
2. Build the solution.

The converters themselves build as plain, portable C++ (`stb_image` / `stb_image_resize2` for image
decoding) — a Linux/GCC build is possible with minor changes if that's ever needed.

## License

Licensed under the **MIT License** — see [LICENSE](LICENSE) for details.
