# FountEngine SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B23)

**FountEngine SDK** is the official content pipeline toolchain for [FountEngine](https://github.com/ImHartash/FountEngine).  
It provides command‑line utilities to convert and package source assets into runtime‑optimized formats consumed directly by the engine.  
The SDK enforces a strict separation between editable source files (e.g., `.png`, `.obj`) and engine‑ready binary data (`.fnttex`, `.fntmdl`, `.fntpk`).

All tools are invoked from the command line. No graphical interface is provided.

---

## Included Tools

| Tool | Input | Output | Status |
|------|-------|--------|--------|
| **fntpk.exe** | Directory of compiled assets | `.fntpk` package | Stable |
| **fnttex.exe** | `.png`, `.jpg`, `.bmp`, `.tga` | `.fnttex` texture | Stable |
| **fntmdl.exe** | Wavefront `.obj` | `.fntmdl` model | Stable |

All tools are self‑contained and link statically against third‑party libraries (`stb_image`, `stb_image_resize2`).

---

## Native Formats

### `.fnttex` — Fount Texture

Produced by **FountEngineTextureConverter**.  
Binary format containing a header followed by raw pixel data ready for Direct3D 11.

**Header (`FNTTEX_HEADER`):**

| Field | Type | Description |
|-------|------|-------------|
| `nMagic` | `uint32_t` | Magic number `'FNTT'` |
| `nVersion` | `uint32_t` | Format version (currently 1) |
| `nWidth` | `uint32_t` | Image width in pixels |
| `nHeight` | `uint32_t` | Image height in pixels |
| `nMipLevels` | `uint32_t` | Number of mip levels |
| `nDXFormat` | `uint32_t` | `DXGI_FORMAT` identifier (default: `DXGI_FORMAT_R8G8B8A8_UNORM`, value 28) |

Pixel data for all mip levels follows the header, from largest to smallest.  
Mipmaps are generated automatically during conversion using `stbir_resize_uint8_srgb`.  
Textures are **uncompressed**.

### `.fntmdl` — Fount Model

Produced by **FountModelConverter**.  
Stores static geometry for rendering.

**Structure:**
1. **Header** (`FNTMDL_HEADER`)
   - Magic number `'FNTM'`
   - Version
   - Vertex count, index count, flags
2. **Vertex array** — Each vertex contains position, normal, texture coordinates, and color (`FNTMDL_VERTEX`)
3. **Index array** — `uint32_t` indices

**Important:** The converter parses **only Wavefront `.obj` files**. Formats such as `.fbx`, `.gltf`, or `.blend` are not supported. Only geometry is transferred — materials and animations are not included.

### `.fntpk` — Fount Package

Produced by **FountEnginePacker**.  
Aggregates multiple compiled assets into a single archive for distribution and loading.

**Header (`FNTPK_HEADER`):**

| Field | Type | Description |
|-------|------|-------------|
| `nMagic` | `uint32_t` | Magic number `'FNTP'` |
| `nVersion` | `uint32_t` | Format version (currently 1) |
| `nFlags` | `uint32_t` | Reserved flags |
| `nCRC` | `uint32_t` | Checksum (reserved) |
| `nBlocksCount` | `uint32_t` | Number of entries |

Each block entry (`FNTPK_BLOCK`) contains:
- `szResourceName[256]` — relative path inside the package
- `nBlockDataOffset` — absolute offset to data
- `nBlockDataSize` — data size in bytes

Files are stored uncompressed for fast seek‑based access.

---

## Usage

### FountEngineTextureConverter

```bash
./fnttex.exe <input_image> [output.fnttex] [max_mip_maps]
```

**Arguments:**
- `input_image` — Path to source image (`.png`, `.jpg`, `.bmp`, `.tga`)
- `output.fnttex` — Optional output path. If omitted, the input filename with `.fnttex` extension is used
- `max_mip_maps` — Optional limit on mip levels. `0` means all possible. If omitted, the full mip chain is generated

**Examples:**
```bash
# Generate all mip levels, save as wood.fnttex
./fnttex.exe wood.png

# Limit to 4 mip levels, specify output path
./fnttex.exe wood.png result.fnttex 4
```

### FountModelConverter

```bash
./fntmdl.exe <input.obj> <output.fntmdl>
```

**Arguments:**
- `input.obj` — Wavefront OBJ file
- `output.fntmdl` — Output path

**Note:** Only `.obj` files are supported. The converter extracts vertices (position, normal, texture coordinates) and triangulates faces.

### FountEnginePacker

```bash
./fntpk.exe <input_directory> <output.fntpk>
```

**Arguments:**
- `input_directory` — Directory containing compiled assets (`.fnttex`, `.fntmdl`)
- `output.fntpk` — Output path for the package

The tool recursively traverses the input directory and packs every file. Resource names are stored as relative paths from the input directory root.

---

## Integration with FountEngine

The intended workflow:

1. Artist creates `character.png` and `level.obj`
2. `FountEngineTextureConverter` converts images to `.fnttex`
3. `FountModelConverter` converts models to `.fntmdl`
4. `FountEnginePacker` bundles everything into `game.fntpk`
5. FountEngine mounts the package and loads assets by resource name

The SDK is distributed separately from the engine and does not require the engine runtime to be installed.

---

## Building from Source

### Prerequisites
- Visual Studio 2022 (v143 toolchain)
- Windows 10 SDK

### Steps
1. Clone the repository: `git clone https://github.com/ImHartash/FountEngine_SDK.git`
2. Open `FountEngine_SDK.sln` in Visual Studio
3. Build the solution (Build → Build Solution)

Alternatively, pre‑compiled binaries are available on the [Releases](https://github.com/ImHartash/FountEngine_SDK/releases) page.

## License

This project is licensed under the terms of the **MIT License**.  
See the [LICENSE](LICENSE) file for details.