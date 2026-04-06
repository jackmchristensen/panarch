# Panarch

A USD asset browser for VFX and 3D pipelines. Panarch scans library folders of USD assets, generates thumbnails, and displays stage metadata like kind, variant sets, composition arcs, prim counts, and more without opening a DCC.

![License](https://img.shields.io/badge/licnse-GPL--3.0-blue)

---

## Features

- **Asset grid**: Browse USD assets as thumbnails with kind chips and indicator dots for variants, payloads, and external dependencies

- **Metadata panel**: View stage-level metadata (up axis, meters per unit, FPS, TCPS, prim count) and composition arcs (sublayers, payloads, references) for any selected asset

- **Variant inspector**: See all variant sets and their selections on the default prim

- **Thumbnail generation**: Renders JPEG thumbnails via an offscreen OpenGL context, cached between sessions

- **Filtering and sorting**: Filter by name or kind, sort by name or modification time

- **DCC integration**: Open assets directly in detected DCCs (usdview, Blender, Houdini, Maya, Katana)

- **Drag and drop**: Drag assets out of Panarch into other applications as file URIs

- **Multiple library roots**: Add any number of scan directories

## Screenshots

*(coming soon)*

---

## Requirements

- Linux (Windows support planned)
- [OpenUSD](https://openusd.org) 0.24.x or later, installed as `uds_m`
- Qt 6.x
- OpenGL

On Arch Linux, dependencies can be installed with:

```bash
sudo pacman -S openusd qt6-base qt6-declarative qt6-5compat
```

---

## Building

Panarch uses CMake and [just](https://github.com/casey/just) as a command runner.

```bash
# Configure and build debug/release (defaults to debug)
just build-configure <mode>

# Run
just run <mode>
```

The build produces four binaries in `build-<mode>/bin/`:

| Binary    | Purpose          |
| --------- | ---------------- |
| `Panarch` | Main application |
| `scan_assets` | Scans a directory and outputs asset metadata as JSON |
| `usd_inspector` | Inspects a single USD saset and outputs stage details as JSON |
| `thumbnail_generator` | Renders a USD asset to a JPEG thumbnail |

`scan_assets`, `usd_inspector`, and `thumbnail_generator` are subprocesses spawned by the main application at runtime. They must be in the same directory as the `Panarch` binary.

### Compiler

Panarch is developed with Clang. GCC should work but is not regularly tested.

```bash
# Override compiler
just cc=gcc cxx=g++ build-configure <mode>
```

## How it Works

Panarch's scan pipeline is split across subprocesses rather than running inside the main application. This is intentional:

- **`scan_assets`** uses `SdfLayer` (not `UsdStage`) to walk the dependency graph cheaply, with out paying the cost of USD composition. It identifies entry point assets by kind (`assembly`, `group`, `component`) and filters out files that are dependencies of other assets rather than standalone entries.

- **`usd_inspector`** opens a full `UsdStage` to read composed metadata like variant sets, stage metrics, the full prim tree, and composition arcs. This is more expensive and runs on demand when an asset is selected.

- **`thumbnail_generator`** uses `UsdImagingGLEngine` with an offscreen OpenGL context via Qt. It runs as a subprocess because initializing Hydra alongside the main Qt UI causes conflicts. Up to 10 thumbnail processes run concurrently while the rest are queued.

---

## Keyboard Shortcuts

| Shortcut | Action |
|---|---|
| `Ctrl+O` | Add library folder |
| `Ctrl+K` / `Ctrl+F` | Focus filter |
| `Ctrl+C` | Copy selected asset path |
| `Ctrl+Q` | Quit|

---

## Asset Detection

Not every USD file in a library is shown. Panarch filters the scan results to surface only meaningful entry points:

- Files with a kind of `assembly`, `group`, or `component` on their default prim are always shown

- Files with no kind and no outbound USD dependencies (standalone files) are shown

- Files that are referenced or payloaded by another asset in the same library are hidden since they're implementation details rather than browsable assets

---

## Project Status

Early alpha. Core functionality works but several known issues exist:

- Library roots list in the UI is not yet implemented (roots are stored in settings but not displayed)
- Prim count is incorrect
- Rapid asset selection can occasionally display details for the wrong asset
- Thumbnail render failures are not distinguished from render successes

---

## License

GPL-3.0. See [LICENSE](LICENSE).