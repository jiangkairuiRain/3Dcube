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
A real‑time, interactive console‑based voxel ray‑tracing engine with advanced rendering features. The program simulates a dynamic 3D voxel world with full camera control, block editing, mirror reflections, custom geometry (triangles, spheres, curved patches), and multiple display modes. It supports both character‑based ASCII rendering and true‑color ANSI output, and includes world saving/loading, screenshot capture, a command system, and a rich set of interaction controls.

### Features
- **Ray‑tracing core** – recursive ray marching with support for:
  - Ordinary voxel blocks (textured cubes with 6 faces, each 16×16 RGB palette)
  - Mirror blocks with planar reflection (including compound mirrors)
  - Special geometry: triangle meshes (Möller–Trumbore), mirror spheres, and textured spherical patches (bilinear surfaces)
  - Dynamic procedural sky with a visible sun – a 300‑second day‑night cycle (`/time` adjusts this). The sun orbits the world; looking directly at it triggers a bright, warm halo, while the rest of the sky displays a smooth HSL‑based gradient that shifts from deep blue to orange/purple hues depending on the sun's altitude.
  - Up to 25 reflection bounces with configurable attenuation (`mirror_reduction = 0.8`)
- **Rendering modes** – switch between:
  - **Character mode** – ASCII symbols representing face directions and block types
  - **True‑color mode** – 24‑bit ANSI pixels with differential updates (only changed pixels redrawn)
- **Projection modes** – toggle between standard perspective and spherical (fisheye) projection
- **World editing** – place/delete blocks, rotate block orientation (48 possible orthogonal transformations), place/delete along reflected directions
- **World management** – save/load entire worlds to/from `./saves`, auto‑backup on load
- **Chunk‑based streaming** – the world is divided into chunks of size 16³ (2^MAX_LOD). The `ensure_chunks()` function runs every frame, automatically saving distant chunks to the `./chunks` folder and unloading them from RAM, while loading nearby chunks on demand. This enables virtually infinite exploration without exhausting memory.
- **Screenshot** – capture both character‑mode text and BMP image (with vertical stretch) to `./images`
- **Command system** – enter commands prefixed with `/` for quick actions:
  - `/give <block_name>` – add a block to the hotbar
  - `/tp <x> <y> <z>` – teleport the player to absolute coordinates (within ±3e7)
  - `/time <seconds>` – set the in‑game time (0–300 seconds cycle)
- **Procedural terrain** – world is generated using 3D Perlin noise (fbm) with a density threshold, creating rolling hills with grass on the surface and stone underneath.

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
├── chunks/                      # On‑disk chunk cache (auto‑managed by ensure_chunks())
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
- **Commands**:
  - `/` – open command prompt; type one of the following and press Enter:
    - `/give <block_name>` – adds the block to the current hotbar slot (or swaps if already assigned)
    - `/tp <x> <y> <z>` – teleports the player to absolute coordinates (limits: ±3e7)
    - `/time <seconds>` – sets the in‑game time (0–300 seconds, wraps around)
  - `ESC` – cancel command input
- **Files**:
  - `C` – capture screenshot (`.txt` + `.bmp` in colour mode)
  - `K` – save current world to `./saves` (timestamped filename)
  - `J` – open load menu (browse saves with `W`/`S`, page up/down with `0`–`9`, `Enter` to load)
  - `` ` `` (backtick) – export world data (all LOD layers) to `2_pow_world.txt` for debugging
  - `ESC` – save world and exit

#### Render Modes
- **Character Mode** (`H` to toggle) – uses directional symbols (`?BFRLDU`) to represent faces; resolution adjustable with `-`/`=`. Minimal overhead, works on any terminal.
- **Colour Mode** – uses 24‑bit ANSI escape sequences. Differential updates redraw only changed pixels, reducing flicker and improving performance. Resolution also adjustable.

#### Projection Modes
- **Standard** – perspective projection with a flat viewport.
- **Spherical** – fisheye‑like projection that maps the view onto a sphere, giving a wide‑angle effect. Toggle with `Y`.

#### World Management & Streaming
- **Saving** (`K`): Saves all placed blocks (type, position, rotation state) to a text file in `./saves` with a timestamp. The world is also automatically backed up before loading another save.
- **Loading** (`J`): Displays a list of save files. Navigate with `W`/`S`; use `0`–`9` to jump pages; press `Enter` to load. If a corresponding screenshot (`.txt`) exists in `./images`, a preview thumbnail is shown.
- **Chunk streaming** (`ensure_chunks()`): The engine maintains a moving window of loaded chunks around the player (configurable radius `DESIRED_CHUNK`). As you move, chunks that fall out of range are automatically serialized to `./chunks` and removed from memory, while newly entered chunks are loaded on‑the‑fly. This allows for seamless, memory‑efficient exploration across vast distances.
- **Screenshots** (`C`): Saves the current frame as a text file (`.txt`) in `./images`. In colour mode, additionally saves a 24‑bit BMP image (vertically stretched for better viewing).

### Configuration
- **Resolution** – nine presets from 64×18 up to 1024×288 (and higher if enabled). Adjust with `-`/`=`.
- **Render Distance** (`view_r`) – ranges from 0.01 to 16.0 (12 steps), adjustable with `[`/`]`. Higher values increase visible range but reduce performance.
- **Hotbar** – customise the 1‑9 shortcuts via the `Tab` menu.
- **Mouse Sensitivity** – fixed at `0.1` degrees per pixel; can be changed in the source.
- **Reflection settings** – `mirror_times = 25` and `mirror_reduction = 0.8` are compile‑time constants.

### Performance Notes
- **Colour mode** uses differential updates – only changed pixels are sent to the console, significantly improving frame rate.
- **Render distance** directly affects the maximum number of ray‑marching steps – reducing it improves performance.
- **Resolution** – lower resolution yields higher FPS; the program automatically handles buffer resizing.
- **Spherical projection** is slightly more computationally intensive than standard.
- **LOD acceleration** (4 levels) speeds up distant voxel queries by aggregating blocks into larger cells.
- **Chunk streaming** (`ensure_chunks()`) keeps the working memory small: only chunks within a fixed radius around the player reside in RAM, while distant areas are stored on disk. This prevents memory bloat even when exploring extremely large worlds.
- **Reflection bounces** – up to 25 bounces with attenuation; reduce `mirror_times` in source for faster rendering.

### Troubleshooting
- **No colour output**: Ensure your terminal supports ANSI escape codes (Windows Terminal or ConEmu recommended). Enable VT processing if needed (the program attempts to enable it automatically).
- **Missing textures**: Verify the `./texture` directory contains `all_world_cube.txt` and the corresponding texture files for each block type. Check file permissions.
- **Low FPS**: Reduce resolution or render distance; consider using character mode.
- **Save/load errors**: Ensure the `./saves` and `./images` directories exist (created automatically on first use) and that the program has write permissions.
- **Mouse not working**: Press `U` to toggle mouse capture; the cursor should be hidden and locked to the console window. If still not working, check that the console window is in focus.
- **Mirror effects not visible**: Make sure you have placed mirror blocks and that the ray bounces are enabled (`mirror_times > 0`).
- **Command not recognised**: Ensure you type the command exactly as shown, with a leading slash. Use `/give grass`, `/tp 0 0 10`, etc.

### Development Notes
- Implements a full ray‑tracing pipeline with recursive reflections.
- Uses DDA‑based voxel traversal for fast AABB stepping.
- Special geometry: triangle meshes (Möller–Trumbore), mirror spheres, and bilinear surface patches (solved via quadratic equations in parameter space).
- 48 orthogonal rotation transformations per block (6 faces × 8 stabilizers) with correct texture mapping.
- LOD system (0–4) for efficient distant world representation.
- **Chunk streaming**: `ensure_chunks()` is called every frame; it calculates the player's current chunk (size 2^MAX_LOD = 16), maintains a surrounding radius of loaded chunks, and automatically triggers save/load operations to/from the `./chunks` directory. This design supports virtually unbounded world exploration with a fixed memory footprint.
- **Dynamic sky with sun**: The `getSkyColor()` function uses a 300‑second cycle (`sky_now_time`) and a sun direction vector that orbits vertically. When the view direction aligns closely with the sun (`cos_theta >= 0.99`), a bright, warm‑colored halo is rendered; otherwise, the sky smoothly transitions through a HSL gradient based on sun altitude, producing realistic dawn, noon, dusk, and night hues.
- All textures are 16×16 RGB grids; special blocks store geometry and texture data.
- World generation uses 3D Perlin noise (fbm) with a density threshold to create hilly terrain; grass is placed on the surface (checked by looking upwards for air), stone fills the interior.
- The program uses a custom vector math library and careful floating‑point rounding to avoid precision errors.
- Colour rendering uses ANSI escape sequences; the output buffer is optimised to minimise console I/O.
- Command parser supports `/give`, `/tp`, `/time`; extendable by modifying `run_command()`.

### Credits
- Ray‑tracing engine with custom vector math and geometric intersection routines.
- Real‑time interactive voxel world with dual rendering (ASCII & colour).
- Special geometry and mirror reflections based on standard algorithms.
- Procedural terrain using Perlin noise (fbm) for natural‑looking landscapes.

---

*For detailed mathematical explanations, see the extensive comments in the source code (the "程序数学原理详解" block).*

---

## License & Attribution
MIT LICENSE

These programs are provided for educational and demonstration purposes. They demonstrate 3D rendering techniques, voxel graphics, and real‑time interaction.

## Support
For issues or questions, please check:
1. All required directories (`texture/`, `chunks/`, `saves/`, `images/`) are present.
2. Texture files follow the correct format (see `all_world_cube.txt` for indexing).
3. Your terminal supports ANSI escape sequences (color mode) and is properly sized.

Enjoy exploring your voxel worlds!