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

## Program 2: Voxel World (Ray‑Traced Voxel Renderer)

### Overview
A real‑time, interactive console‑based voxel ray‑tracing engine with advanced rendering features. The program simulates a dynamic 3D voxel world with full camera control, block editing, mirror reflections, custom geometry (triangles, spheres, curved patches), and multiple display modes. It supports both character‑based ASCII rendering and true‑color ANSI output, and includes world saving/loading, screenshot capture, and a rich set of interaction controls.

### Features
- **Ray‑tracing core** – recursive ray marching with support for:
  - Ordinary voxel blocks (textured cubes with 6 faces, each 16×16 RGB palette)
  - Mirror blocks with planar reflection (including compound mirrors)
  - Special geometry: triangle meshes (Möller–Trumbore), mirror spheres, and textured spherical patches (bilinear surfaces)
  - Dynamic procedural sky (HSL‑based, time‑varying)
  - Up to 25 reflection bounces with configurable attenuation (`mirror_reduction = 0.8`)
- **Rendering modes** – switch between:
  - **Character mode** – ASCII symbols representing face directions and block types
  - **True‑color mode** – 24‑bit ANSI pixels with differential updates (only changed pixels redrawn)
- **Projection modes** – toggle between standard perspective and spherical (fisheye) projection
- **World editing** – place/delete blocks, rotate block orientation (48 possible orthogonal transformations), place/delete along reflected directions
- **World management** – save/load entire worlds to/from `./saves`, auto‑backup on load
- **Screenshot** – capture both character‑mode text and BMP image (with vertical stretch) to `./images`
- **Performance** – adjustable render distance (0.01–16.0), resolution (9 presets up to 1024×288), and LOD acceleration (0–4)
- **Mouse support** – drag to look, left‑click to delete, right‑click to place (toggleable capture)

### Requirements
- Windows operating system (10 or later) with console support for ANSI escape sequences (VT processing)
- C++ compiler with C++17 support (GCC/MinGW recommended)
- Required headers: `<bits/stdc++.h>`, `<windows.h>`, `<conio.h>`, `<sys/time.h>`, `<dirent.h>`, `<sys/stat.h>`

### Installation & Compilation
```bash
# Compile with g++ (optimised)
g++ -static -std=c++17 -O3 -o VoxelWorld.exe VoxelWorld.cpp
```

### File Structure
```
3Dcube/
├── VoxelWorld.cpp               # Main program
├── texture/                     # Texture definitions
│   ├── all_world_cube.txt       # Index of all block types (including special ones)
│   ├── grass.txt                # 6×16×16 RGB texture for grass block
│   ├── stone.txt                # Texture for stone block
│   ├── mirror.txt               # Mirror block (planar reflection)
│   └── ...                      # Other block textures and special geometry files
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
  - `Q` / `E` – rotate left/right (yaw, ±2° step)
  - `R` / `F` – look up/down (pitch, ±2° step)
  - Mouse drag (when captured) – free look with adjustable sensitivity
- **Block Interaction**:
  - `B` – place selected block in front of the player (on the face you are looking at)
  - `V` – delete the block the player is looking at
  - `X` – place block along the reflection direction (useful for placing mirrors)
  - `Z` – delete block along the reflection direction
  - `N` – rotate the gazed block’s face orientation (changes which face is “front”)
  - `M` – rotate the gazed block’s spin (in‑plane rotation) – both give 48 total orientations
- **Block Selection**:
  - `1`–`9` – select a block from the hotbar (default: 1:grass, 2:stone, 3:mirror, etc.)
  - `Tab` – open hotbar mapping menu: navigate with `W`/`S`, assign a block to a number key by pressing the desired digit
- **System**:
  - `H` – toggle between character and colour rendering modes
  - `Y` – toggle between standard and spherical projection
  - `U` – toggle mouse capture/release (cursor is hidden and locked to console window)
  - `-` / `=` – decrease/increase resolution (64×18 to 1280×360)
  - `[` / `]` – decrease/increase render distance (0.01 to 16.0, 12 steps)
  - `C` – capture screenshot (`.txt` + `.bmp` in colour mode)
  - `K` – save current world to `./saves` (timestamped filename)
  - `J` – open load menu (browse saves with `W`/`S`, page up/down with `0`–`9`, `Enter` to load)
  - `\` (backtick) – export world data (all LOD layers) to `2_pow_world.txt` for debugging
  - `ESC` – save world and exit

#### Render Modes
- **Character Mode** (`H` to toggle) – uses directional symbols (`?BFRLDU`) to represent faces; resolution adjustable with `-`/`=`. Minimal overhead, works on any terminal.
- **Colour Mode** – uses 24‑bit ANSI escape sequences. Differential updates redraw only changed pixels, reducing flicker and improving performance. Resolution also adjustable.

#### Projection Modes
- **Standard** – perspective projection with a flat viewport.
- **Spherical** – fisheye‑like projection that maps the view onto a sphere, giving a wide‑angle effect. Toggle with `Y`.

#### World Management
- **Saving** (`K`): Saves all placed blocks (type, position, rotation state) to a text file in `./saves` with a timestamp. The world is also automatically backed up before loading another save.
- **Loading** (`J`): Displays a list of save files. Navigate with `W`/`S`; use `0`–`9` to jump pages; press `Enter` to load. If a corresponding screenshot (`.txt`) exists in `./images`, a preview thumbnail is shown.
- **Screenshots** (`C`): Saves the current frame as a text file (`.txt`) in `./images`. In colour mode, additionally saves a 24‑bit BMP image (vertically stretched for better viewing).

### Configuration
- **Resolution** – nine presets from 64×18 up to 1024×288 (and higher if enabled). Adjust with `-`/`=`.
- **Render Distance** (`view_r`) – ranges from 0.01 to 16.0 (12 steps), adjustable with `[`/`]`. Higher values increase visible range but reduce performance.
- **Hotbar** – customise the 1‑9 shortcuts via the `Tab` menu.
- **Mouse Sensitivity** – fixed at `0.1` degrees per pixel; can be changed in the source.

### Performance Notes
- **Colour mode** uses differential updates – only changed pixels are sent to the console, significantly improving frame rate.
- **Render distance** directly affects the maximum number of ray‑marching steps – reducing it improves performance.
- **Resolution** – lower resolution yields higher FPS; the program automatically handles buffer resizing.
- **Spherical projection** is slightly more computationally intensive than standard.
- **LOD acceleration** (Level of Detail, 0–4) speeds up distant voxel queries by aggregating blocks into larger cells.
- **Reflection bounces** – up to 25 bounces with attenuation; reduce `mirror_times` in source for faster rendering.

### Troubleshooting
- **No colour output**: Ensure your terminal supports ANSI escape codes (Windows Terminal or ConEmu recommended). Enable VT processing if needed (the program attempts to enable it automatically).
- **Missing textures**: Verify the `./texture` directory contains `all_world_cube.txt` and the corresponding texture files for each block type. Check file permissions.
- **Low FPS**: Reduce resolution or render distance; consider using character mode.
- **Save/load errors**: Ensure the `./saves` and `./images` directories exist (created automatically on first use) and that the program has write permissions.
- **Mouse not working**: Press `U` to toggle mouse capture; the cursor should be hidden and locked to the console window. If still not working, check that the console window is in focus.
- **Mirror effects not visible**: Make sure you have placed mirror blocks and that the ray bounces are enabled (`mirror_times > 0`).

### Development Notes
- Implements a full ray‑tracing pipeline with recursive reflections.
- Uses DDA‑based voxel traversal for fast AABB stepping.
- Special geometry: triangle meshes (Möller–Trumbore), mirror spheres, and bilinear surface patches (solved via quadratic equations in parameter space).
- 48 orthogonal rotation transformations per block (6 faces × 8 stabilizers) with correct texture mapping.
- LOD system (0–4) for efficient distant world representation.
- All textures are 16×16 RGB grids; special blocks store geometry and texture data.
- World generation uses 3D Perlin noise (fbm) to create a hilly terrain with grass on the surface and stone below.
- The program uses a custom vector math library and careful floating‑point rounding to avoid precision errors.
- Colour rendering uses ANSI escape sequences; the output buffer is optimised to minimise console I/O.

### Credits
- Ray‑tracing engine with custom vector math and geometric intersection routines.
- Real‑time interactive voxel world with dual rendering (ASCII & colour).
- Special geometry and mirror reflections based on standard algorithms.

---

*For detailed mathematical explanations, see the extensive comments in the source code (the "程序数学原理详解" block).*


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

