# 3D Graphics Rendering Programs - English User Guide

## Program 1: CubeRenderer (3D Cube Rendering Engine)

### Overview
A real-time 3D cube rendering engine that displays a textured cube with interactive camera controls. The program supports both character-based and colored rendering modes with texture mapping.

### Features
- Real-time 3D cube rendering with texture mapping
- Multiple rendering modes: character mode and optimized color mode
- Interactive camera control (rotation around all axes)
- Support for custom textures
- FPS counter and performance monitoring
- Adjustable screen resolution

### Requirements
- Windows operating system
- C++ compiler with C++14 support
- Required headers: `<bits/stdc++.h>`, `<windows.h>`, `<conio.h>`, `<sys/time.h>`

### Installation & Compilation
```bash
# Compile with g++
g++ -o CubeRenderer.exe CubeRenderer.cpp -std=c++14
```

### File Structure
```
3Dcube/
├── CubeRenderer.cpp    # Main program file
├── texture/           # Texture directory
│   ├── grass1.txt    # Grass texture (front face)
│   ├── grass2.txt    # Grass texture (back face)
│   ├── grass3.txt    # Grass texture (right face)
│   ├── grass4.txt    # Grass texture (left face)
│   ├── grass5.txt    # Grass texture (bottom face)
│   └── grass6.txt    # Grass texture (top face)
└── stone[1-6].txt    # Stone textures (same structure)
```

### How to Use
1. **Compile and run** the program
2. **Choose rendering mode** when prompted:
   - Press `1` for character mode (ASCII rendering)
   - Press `2` for optimized color mode (requires terminal color support)

3. **Camera Controls**:
   - `Q/E` - Rotate around Y-axis (yaw)
   - `A/D` - Rotate around P-axis (pitch)
   - `W/S` - Rotate around R-axis (roll)
   - `ESC` - Exit program

4. **Display Information**:
   - Current rotation angles (Y/P/R)
   - Screen resolution
   - Real-time FPS counter

### Texture Format
Textures should be 16×16 grids of integer RGB values in decimal format:
```
255 128 64  255 128 64  ...
255 128 64  255 128 64  ...
... (16 lines total)
```

### Performance Tips
- Character mode provides better performance on systems without color support
- Optimized color mode uses terminal escape sequences for better visual quality
- Frame rate may vary based on screen resolution and system performance

---

## Program 2: Voxel World (Ray-Traced Voxel Renderer)

### Overview
A real-time, interactive console-based voxel ray-tracing engine with advanced rendering features. The program simulates a dynamic 3D voxel world with full camera control, block editing, mirror reflections, custom geometry (triangles, spheres, curved patches), and multiple display modes. It supports both character‑based ASCII rendering and true‑color ANSI output, and includes world saving/loading, screenshot capture, and a rich set of interaction controls.

### Features
- **Ray‑tracing core** – recursive ray marching with support for:
  - Ordinary voxel blocks (textured cubes with 6 faces)
  - Mirror blocks with planar reflection
  - Special geometry: triangle meshes, mirror spheres, textured spherical patches (bilinear surfaces)
  - Dynamic procedural sky (HSL‑based, time‑varying)
  - Multiple reflection bounces with attenuation
- **Rendering modes** – switch between:
  - **Character mode** – ASCII symbols representing faces and materials
  - **True‑color mode** – 24‑bit ANSI pixels with differential updates (only changed pixels redrawn)
- **Projection modes** – toggle between standard perspective and spherical (fisheye) projection
- **World editing** – place/delete blocks, rotate block orientation (48 possible transformations), place/delete along reflected directions
- **World management** – save/load entire worlds to/from `./saves`, auto‑backup on load
- **Screenshot** – capture both character‑mode text and BMP image (with vertical stretch) to `./images`
- **Performance** – adjustable render distance, resolution, and LOD acceleration
- **Mouse support** – drag to look, left‑click to delete, right‑click to place (toggleable capture)

### Requirements
- Windows operating system (10 or later) with console support for ANSI escape sequences
- C++ compiler with C++17 support (e.g., GCC/MinGW)
- Required headers: `<bits/stdc++.h>`, `<windows.h>`, `<conio.h>`, `<sys/time.h>`, `<dirent.h>`, `<sys/stat.h>`
- Optional: OpenCL SDK for GPU acceleration (compile with `-DUSE_OPENCL`)

### Installation & Compilation
```bash
# Compile with g++ (CPU only)
g++ -static -std=c++17 -O3 -o VoxelWorld.exe VoxelWorld.cpp

# Compile with OpenCL support (GPU acceleration)
g++ -static -std=c++17 -O3 -DUSE_OPENCL -o VoxelWorld.exe VoxelWorld.cpp -lOpenCL
```

### File Structure
```
3Dcube/
├── VoxelWorld.cpp               # Main program
├── texture/                     # Texture definitions
│   ├── all_world_cube.txt       # Index of all block types (including special ones)
│   ├── grass.txt                # 6×16×16 RGB texture for grass block
│   ├── stone.txt                # Texture for stone block
│   ├── mirror.txt               # Mirror block
│   └── ...                      # Other block textures (special geometry files)
├── saves/                       # World save files (created automatically)
├── images/                      # Screenshots (character .txt and .bmp)
└── README.md                    # This documentation
```

### How to Use

#### Basic Controls
- **Movement**:
  - `W` / `S` – move forward/backward (along viewing direction)
  - `A` / `D` – strafe left/right
  - `P` / `L` – move up/down (vertical)
  - `T` / `G` – move forward/backward *horizontally* (ignoring pitch)
- **Camera**:
  - `Q` / `E` – rotate left/right (yaw)
  - `R` / `F` – look up/down (pitch)
  - Mouse drag (when captured) – free look
- **Block Interaction**:
  - `B` – place selected block in front of the player
  - `V` – delete the block the player is looking at
  - `X` – place block along the reflection direction (for mirror placement)
  - `Z` – delete block along the reflection direction
  - `N` / `M` – rotate the gazed block’s orientation / spin (48 possible orientations)
- **Block Selection**:
  - `1`–`9` – select a block from the hotbar (assigned via `Tab` menu)
  - `Tab` – open hotbar mapping menu: navigate with `W`/`S`, assign a block to a number key
- **System**:
  - `H` – toggle between character and color rendering modes
  - `Y` – toggle between standard and spherical projection
  - `U` – toggle mouse capture/release
  - `-` / `=` – decrease/increase resolution (affects both modes)
  - `[` / `]` – decrease/increase render distance
  - `C` – capture screenshot (`.txt` + `.bmp` in color mode)
  - `K` – save current world to `./saves`
  - `J` – open load menu (browse saves with `W`/`S`, `Enter` to load)
  - `` ` `` (backtick) – export world data (all LOD layers) to `2_pow_world.txt`
  - `ESC` – exit program

### Render Modes
- **Character Mode** (`H` to toggle) – displays blocks using directional symbols (`?BFRLDU`). Resolution can be adjusted with `-`/`=`. Suitable for low‑performance terminals.
- **Color Mode** – uses 24‑bit ANSI escape sequences for full‑color display. Optimized with differential updates – only changed pixels are redrawn, reducing flicker and increasing performance. Resolution is also adjustable.

### Projection Modes
- **Standard** – ordinary perspective projection (straight lines remain straight).
- **Spherical** – fisheye‑like projection, creating a wide‑angle view effect. Toggle with `Y`.

### World Management
- **Saving** (`K`): Saves all placed blocks, their types, and rotation states to a timestamped file in `./saves`. The world is also automatically backed up before loading another save.
- **Loading** (`J`): Displays a list of available save files. Navigate with `W`/`S`; press `Enter` to load. A preview thumbnail (character‑mode screenshot) is shown if available.
- **Screenshots** (`C`): Saves the current frame as a text file (`.txt`) in `./images`. In color mode, additionally saves a 24‑bit BMP image (vertically stretched for better viewing).
- **World Export** (`` ` ``): Dumps all LOD data into a plain text file for debugging.

### Configuration
- **Resolution**  adjustable for both character and color modes via -/= keys. Presets range from 64×18 up to 1024×288 (and higher if enabled). Lower resolution increases performance.
- **Render Distance** (`view_r`): Adjustable from 0.01 to 16.0 using `[`/`]`. Higher values increase visibility but reduce performance.
- **Hotbar**: Customize the 1‑9 block shortcuts via the `Tab` menu.

### Performance Notes
- **Color mode** uses differential updates – only changed pixels are sent to the console, significantly improving frame rate.
- **Render distance** affects ray‑marching steps – reducing it improves performance.
- **Resolution** directly impacts pixel count – lower resolution yields higher FPS.
- **Spherical projection** is slightly more computationally intensive than standard.
- **LOD acceleration** (Level of Detail, 0–4) speeds up distant voxel queries.
- **OpenCL acceleration** (if compiled with `-DUSE_OPENCL`) offloads ray‑tracing to the GPU, providing a major performance boost on supported hardware.

### Troubleshooting
- **No color output**: Ensure your console supports ANSI escape codes (Windows Terminal or ConEmu recommended). Enable VT processing if needed.
- **Missing textures**: Verify the `./texture` directory contains `all_world_cube.txt` and the corresponding texture files for each block type.
- **Low FPS**: Reduce resolution or render distance; consider using character mode or enabling OpenCL.
- **Save/load errors**: Ensure the `./saves` and `./images` directories exist (they are created automatically on first use) and that the program has write permissions.
- **Mouse not working**: Press `U` to toggle mouse capture; the mouse cursor will be hidden and locked to the console window.

### Development Notes
- Implements a full ray‑tracing pipeline with recursive reflections.
- Uses DDA‑based voxel traversal for fast AABB stepping.
- Supports special geometry (triangles, spheres, bilinear patches) via Möller–Trumbore and custom solving routines.
- 48 orthogonal rotation transformations per block (6 faces × 8 stabilizers) with correct texture mapping.
- LOD system (0–4) for efficient distant world representation.
- All textures are 16×16 RGB grids.
- The world is procedurally generated on startup with a Perlin‑noise based terrain (grass and stone).

### Credits
- Ray‑tracing engine with custom vector math and geometric intersection routines.
- Real‑time interactive voxel world with dual rendering (ASCII & color).
- Special geometry and mirror reflections.
- OpenCL support for GPU acceleration.

---

## License & Attribution
MIT LICENSE

These programs are provided for educational and demonstration purposes. They demonstrate 3D rendering techniques, voxel graphics, and real‑time interaction.

## Support
For issues or questions, please check:
1. All required directories (`texture/`, `saves/`, `images/`) are present.
2. Texture files follow the correct format (see `all_world_cube.txt` for indexing).
3. Your terminal supports ANSI escape sequences (color mode) and is properly sized.

Enjoy exploring your voxel worlds!

