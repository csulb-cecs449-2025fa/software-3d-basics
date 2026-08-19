# Basics of software 3D rendering

A series of C++ projects using CMake, vcpkg, SFML, GLM, Assimp, and a small
software rasterizer. Each project builds on the previous one, following the
lecture progression from screen coordinates to textured 3D meshes.

## Rendering progression

The stages are intentionally separate. Similar code is repeated so that each
lecture can introduce one new coordinate-space or pipeline idea without hiding
the earlier steps in a shared abstraction.

- **Static2D** — screen-space vertices and indexed triangle meshes.
- **ClipSpace** — normalized device coordinates mapped to the screen.
- **Vertex3D** — adds a z coordinate before introducing camera transforms.
- **ViewSpace** — view-space vertices projected through a frustum.
- **WorldSpace** — camera position and orientation in world space.
- **LocalSpace** — per-object local-to-world position, orientation, and scale.
- **Matrices** — GLM model, view, and projection matrix pipeline.
- **Assimp** — loads the Stanford Bunny from an OBJ file.
- **Textures** — rasterizes filled triangles with depth testing, perspective-
  correct UV interpolation, and image texture sampling.

The coordinate transforms, mesh loading, triangle filling, depth handling, and
texture mapping remain in their respective stages. Nothing from those lecture
steps has been moved into `Common`.

## Common raster support

`Common` contains only the storage types shared by the stages:

- `Pixel` — an RGBA pixel with readable GTest diagnostics.
- `Framebuffer` — a CPU-side, row-major pixel buffer.

`Common` also contains the identical wireframe primitives shared by all
stages:

- `drawLine` — integer Bresenham line drawing.
- `drawTriangle` — a wireframe triangle made from three Bresenham lines.

The lecture-specific coordinate transforms, mesh loading, triangle filling,
depth handling, and texture mapping remain in their respective stages.

The applications render into a `Framebuffer`, upload its RGBA bytes once per
frame to an SFML texture, and draw one SFML sprite. This keeps the software
rasterizer deterministic and makes the pixels directly testable without
opening an SFML window.

## Building

Install vcpkg and set `VCPKG_ROOT` before configuring. The manifest installs
SFML, GLM, Assimp, libpng, and GoogleTest.

On Windows x64:

```powershell
cmake --preset x64-debug
cmake --build --preset x64-debug
ctest --preset x64-debug
```

On Windows ARM64, use `windows-arm64-debug`. On macOS, use
`macos-arm64-debug` or `macos-x64-debug` as appropriate. Release presets
are available with the same names ending in `-release`.

The Windows x64 presets are hidden on Windows ARM machines, and the Windows
ARM64 presets are hidden on Windows x64 machines. The macOS presets are
available for either macOS host so that an instructor can intentionally build
a cross-architecture target.

The test executable is `software_3d_raster_tests`. It checks framebuffer
storage and the shared line/triangle primitives using exact expected pixel
sets. The tests do not depend on SFML's rendering output.
