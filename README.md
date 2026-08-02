<div align="center">

# FountEngine SDK

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
[![C++](https://img.shields.io/badge/C%2B%2B-23-blue.svg?style=for-the-badge)](https://en.wikipedia.org/wiki/C%2B%2B23)

</div>

**FountEngine SDK** is the content pipeline for [FountEngine](https://github.com/ImHartash/FountEngine). It converts human-editable source assets — images, Wavefront `.obj` models, plain-text material descriptions — into the compact binary formats the engine loads at runtime. The engine never reads source formats directly; it only reads what this SDK produces.

The split mirrors a common pattern in commercial engines (Source 2's content/game-side compiled resources, for instance): you never hand-edit or ship the binary formats — you edit the source, and the SDK compiles it.

`Русская версия — в конце файла →` [Перейти](#русская-версия)

---

## Table of Contents

- [Command-Line Converters](#command-line-converters)
- [Binary Formats](#binary-formats)
- [SdkTool (GUI)](#sdktool-gui--this-branch)
  - [What it does](#what-it-does)
  - [Component breakdown](#component-breakdown)
- [Building](#building)
- [Typical Workflow](#typical-workflow)
- [License](#license)
- [Русская версия](#русская-версия)

---

## Command-Line Converters

| Tool | Input | Output |
| --- | --- | --- |
| `fntmdl` | Wavefront `.obj` | `.fntmdl` — mesh geometry |
| `fnttex` | `.png`, `.jpg` | `.fnttex` — uncompressed GPU texture |
| `fntmat` | Plain-text material description (`.txt`) | `.fntmat` — compiled material |
| `fntpk` | A directory of compiled assets | `.fntpk` — package archive |

```bash
fntmdl <input.obj> <output.fntmdl>
fnttex <input_image> [output.fnttex] [max_mip_maps]
fntmat <input_descriptor.txt> [output.fntmat]
fntpk  <input_directory> <output.fntpk>
```

`fnttex` and `fntmat` infer the output path from the input filename if it's omitted. `fntpk` walks the input directory recursively and packs every file it finds, using paths relative to the directory root as resource names.

A `.fntmat` text descriptor looks like this:

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

## Binary Formats

All headers use `#pragma pack(push, 1)` for a stable on-disk layout, and are shared verbatim between the converters, `SdkTool`, and the engine.

**`.fntmdl`** — `FNTMDL_HEADER` (magic `FNTM`, version, vertex/index counts, offsets, flags) followed by a vertex array (position, normal, UV) and a `uint32_t` index array. Static geometry only — no skinning or animation, and only `.obj` is accepted as input.

**`.fnttex`** — `FNTTEX_HEADER` (magic `FNTT`, version, width, height, mip levels, `DXGI_FORMAT`) followed by raw pixel data per mip level, largest first. Stored uncompressed (`DXGI_FORMAT_R8G8B8A8_UNORM` = `28`) — no block compression yet.

**`.fntmat`** — `FNTMAT_HEADER` (magic `FTMT`, version) + a data block with the diffuse texture path, ambient/diffuse/specular colors, shininess, opacity, and three enums: blend mode (`Opaque` / `AlphaBlend` / `Additive`), cull mode (`Back` / `Front` / `None`), depth mode (`Enabled` / `Disabled` / `ReadOnly`).

**`.fntpk`** — `FNTPK_HEADER` (magic `FNTP`, version, flags, CRC, block count) followed by an array of `FNTPK_BLOCK` entries (256-byte resource name, data offset, data size), then the raw concatenated file data. Uncompressed, seek-based — not a general-purpose archive format.

## SdkTool (GUI)

`SdkTool` is a Qt 6 Widgets application (built as the `sdk_tool` executable) that wraps the four command-line converters in a project/asset browser, so assets can be converted, previewed, and packed without touching a terminal.

### What it does

- **Project tree** (`CAssetFileSystemModel` + `CAssetTreeView`) — a filtered file-system view rooted at your project folder, showing only `.obj`, `.png`, `.jpg`, `.fntmdl`, `.fnttex`, `.fntmat`, `.fntpk`. Supports right-click **New Folder**, **Rename**, **Delete**, and **New Material...**.
- **Drag-and-drop auto-conversion** — dropping a `.obj`/`.png`/`.jpg` file (or several) onto the tree queues it for conversion with the matching CLI tool (`CConverterRunner`, a thin `QProcess` wrapper) and copies any other file type in as-is. Conversions run one at a time from a `QQueue`; each result is logged in-app and remembered (source name + timestamp) so re-selecting the output shows *"Auto-converted from X · N min ago"*.
- **Asset preview panel** — selecting a file shows a generated icon (or, for `.fnttex`, a decoded thumbnail read directly from the mip-0 pixel data) plus metadata: vertex/triangle count for `.fntmdl`, dimensions and mip count for `.fnttex`, file size as a fallback for anything else.
- **Material creator** (`CMaterialCreatorDialog`) — a form (name, diffuse texture browse button, ambient/diffuse/specular color spinboxes, shininess, opacity, blend/cull/depth mode dropdowns) that writes a `.fntmat` text descriptor and invokes `fntmat` on it directly, without hand-editing text.
- **Package explorer** (`CPackExplorerDialog`, opened by double-clicking a `.fntpk`) — lists the entries inside an existing package and supports adding new files or removing existing entries, then re-saving the archive in place.
- **Settings** (`CSettingsDialog` / `CSettingsManager`, `Settings...` in the menu bar) — persists (via `QSettings`, org `FountEngine`, app `SdkTool`) the converters folder, the project folder, and the pack output name/directory. A first-run check (`CheckFirstRun`) prompts for these if either is missing.
- **Pack button** — runs `fntpk` against the whole project folder into the configured output path in one click.

### Component breakdown

| Class | Responsibility |
| --- | --- |
| `MainWindow` | Owns the whole UI, the conversion queue, and wiring between components. |
| `CAssetFileSystemModel` | `QFileSystemModel` subclass; appends `/` to directory names. |
| `CAssetTreeView` | `QTreeView` subclass; drag/drop, context menu, delete-key handling. |
| `CConverterRunner` | Runs a converter executable via `QProcess`, streams stdout as log lines, reports exit code. |
| `CFileIconProvider` | Hand-drawn `QPainter` icons per asset type (cube, image, material, archive, folder, check/error marks) — no icon files on disk. |
| `CMaterialCreatorDialog` | Form → `.fntmat` text descriptor → invokes `fntmat`. |
| `CPackExplorerDialog` | Reads a `.fntpk`'s block table; add/remove entries; re-saves. |
| `CSettingsDialog` / `CSettingsManager` | Persisted tool configuration. |
| `CFntFormats.hpp` | A local copy of the four format headers, kept in sync with the converters' own headers. |

## Building

### Prerequisites

- CMake 3.16+
- A C++20 compiler (MSVC, GCC, or Clang) — the converters target C++23, `SdkTool` targets C++20
- Qt 6 (`Widgets` component) — required only for `SdkTool`; the four converters have no Qt dependency

### Steps

```bash
git clone https://github.com/ImHartash/FountEngine_SDK.git
cd FountEngine_SDK
git checkout feature/sdk_tool
mkdir build && cd build
cmake ..
cmake --build .
```

All resulting binaries (`fntmdl`, `fnttex`, `fntmat`, `fntpk`, `sdk_tool`) land under `bin/` inside the build directory. If CMake can't find Qt automatically:

```bash
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" ..
```

On Windows, the post-build step copies the required Qt DLLs (`Qt6Core`, `Qt6Widgets`, `Qt6Gui`) and the `platforms` plugin directory next to `sdk_tool.exe` automatically.

## Typical Workflow

**Command line:**
1. Author source assets: `character.png`, `level.obj`, a `.fntmat` text descriptor per material.
2. Run each through the matching converter.
3. Pack the resulting `.fntmdl` / `.fnttex` / `.fntmat` files into a `.fntpk` with `fntpk`.
4. Point FountEngine at the package; it loads assets by resource name.

**With SdkTool:**
1. Set converters folder and project folder in **Settings** on first run.
2. Drag `.obj`/`.png`/`.jpg` files into the project tree — conversion runs automatically.
3. Right-click → **New Material...** to build `.fntmat` files from a form.
4. Click **Pack to .fntpk** to bundle the whole project into the configured output package.
5. Double-click any `.fntpk` to inspect or edit its contents without a full repack.

## License

Licensed under the **MIT License** — see [LICENSE](LICENSE) for details.

---

## Русская версия

**FountEngine SDK** — конвейер контента для [FountEngine](https://github.com/ImHartash/FountEngine). Преобразует редактируемые исходные ассеты (изображения, модели `.obj`, текстовые описания материалов) в компактные бинарные форматы, которые движок загружает в рантайме. Сам движок никогда не читает исходные форматы напрямую — только то, что производит SDK.

> Этот README описывает ветку `feature/sdk_tool`. Она добавляет GUI-инструмент `SdkTool` на Qt 6 поверх четырёх существующих консольных конвертеров; сами конвертеры не изменились относительно `main`.

### Консольные инструменты

| Инструмент | Вход | Выход |
| --- | --- | --- |
| `fntmdl` | Wavefront `.obj` | `.fntmdl` — геометрия |
| `fnttex` | `.png`, `.jpg` | `.fnttex` — несжатая текстура |
| `fntmat` | Текстовое описание материала | `.fntmat` — скомпилированный материал |
| `fntpk` | Папка со скомпилированными ассетами | `.fntpk` — архив-пакет |

### SdkTool (GUI)

Qt-приложение (`sdk_tool`), которое оборачивает четыре консольных конвертера в браузер ассетов:

- Дерево проекта с drag-and-drop автоконвертацией — перетащил `.obj`/`.png`/`.jpg`, файл сам сконвертировался нужным инструментом.
- Панель предпросмотра: количество вершин/треугольников для `.fntmdl`, размеры и число мип-уровней для `.fnttex`, миниатюра текстуры, декодированная прямо из пиксельных данных.
- Форма создания материала — `.fntmat` собирается без ручного редактирования текста.
- Просмотрщик `.fntpk` (двойной клик по пакету) — добавление/удаление файлов внутри архива без полной пересборки.
- Настройки (папка конвертеров, папка проекта, путь и имя пакета), сохраняются между запусками через `QSettings`.

**Известные ограничения этой ветки:** путь к Qt захардкожен в `SdkTool/CMakeLists.txt`; локальная копия заголовков форматов в `CFntFormats.hpp` расходится с движком по разрядности `nBlockDataOffset` (`uint32_t` вместо актуального `uint64_t`) — стоит поправить перед мерджем; живой предпросмотр моделей и процедурная генерация ассетов в инструмент пока не входят.

### Сборка

Нужны CMake 3.16+, компилятор с поддержкой C++20 (для `SdkTool`, конвертеры используют C++23), и Qt 6 (компонент `Widgets`) — только для `SdkTool`.

```bash
git clone https://github.com/ImHartash/FountEngine_SDK.git
cd FountEngine_SDK
git checkout feature/sdk_tool
mkdir build && cd build
cmake ..
cmake --build .
```

Если CMake не находит Qt автоматически:

```bash
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2022_64" ..
```

### Лицензия

Проект распространяется под лицензией **MIT** — см. [LICENSE](LICENSE).
