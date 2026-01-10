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
- C++ compiler with C++11 support
- Required headers: `<bits/stdc++.h>`, `<windows.h>`, `<conio.h>`, `<sys/time.h>`

### Installation & Compilation
```bash
# Compile with g++
g++ -o CubeRenderer.exe CubeRenderer.cpp -std=c++11
```

### File Structure
```
Project/
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

## Program 2: Voxel World (Minecraft-style Renderer)

### Overview
A first-person voxel rendering engine inspired by Minecraft. Allows users to explore a 3D world, place and destroy blocks, with support for multiple rendering modes and texture packs.

### Features
- First-person voxel world exploration
- Multiple block types: grass, stone, test blocks
- Two rendering modes: character-based and colored
- Two projection modes: standard and spherical (fisheye effect)
- Block placement/destruction system
- World saving/loading functionality
- Screenshot capture capability
- Adjustable render distance and resolution

### Requirements
- Windows operating system
- C++ compiler with C++11 support
- Terminal with ANSI color support (for color mode)
- Required directories: `./texture`, `./saves`, `./images`

### Installation & Compilation
```bash
# Compile with g++
g++ -o VoxelWorld.exe VoxelWorld.cpp -std=c++11
```

### File Structure
```
Project/
├── VoxelWorld.cpp     # Main program file
├── texture/           # Texture packs
│   ├── grass1.txt    # Grass block textures
│   ├── stone1.txt    # Stone block textures
│   └── ...
├── saves/             # World save files
├── images/            # Screenshot saves
└── README.md          # This documentation
```

### How to Use

#### Basic Controls
1. **Movement**:
   - `W/S` - Move forward/backward
   - `A/D` - Strafe left/right
   - `P/L` - Move up/down (vertical movement)
   - `T/G` - Move along horizontal plane (independent of pitch)

2. **Camera Control**:
   - `Q/E` - Look left/right (yaw)
   - `R/F` - Look up/down (pitch)

3. **Block Interaction**:
   - `B` - Place block in front of player
   - `V` - Destroy block in front of player
   - `1-9` - Select block type (check idcube array for available types)

4. **System Controls**:
   - `H` - Toggle between character and color rendering modes
   - `Y` - Toggle between standard and spherical projection
   - `-`/`=` - Decrease/Increase resolution (character mode only)
   - `[`/`]` - Decrease/Increase render distance (r value)
   - `C` - Take screenshot (saves to images folder)
   - `K` - Save world state (saves to saves folder)
   - `J` - Load world from save file
   - `ESC` - Exit program

### Render Modes

#### Character Mode
- Uses ASCII characters to represent blocks
- Higher resolution options available
- Better performance on all terminals

#### Color Mode
- Uses 24-bit true color via ANSI escape codes
- Smooth texture rendering
- Requires terminal with color support

### Projection Modes

#### Standard Projection
- Traditional 3D perspective
- Straight lines remain straight

#### Spherical Projection
- Fisheye-like spherical projection
- Creates wide-angle view effect
- Toggle with `Y` key

### World Management

#### Saving Worlds
- Press `K` to save current world
- Saves include:
  - All placed blocks and their positions
  - Block types and orientations
  - Timestamp-based filename

#### Loading Worlds
1. Press `J` to open load menu
2. Navigate with `W`/`S` keys
3. Press `Enter` to load selected world
4. Current world is automatically backed up if modified

#### Screenshots
- Press `C` to capture screenshot
- Saves ASCII representation to images folder
- Timestamp-based filenames

### Configuration

#### Resolution Settings (Character Mode Only)
- Resolution presets:
  - 64×18, 128×36, 192×54, 384×108, 512×144, 1024×288
- Higher resolution = more detail, lower performance

#### Render Distance (r value)
- Adjustable from 0.1 to 16.0
- Higher values = further visibility
- Affects performance significantly

### Performance Notes
- **Color Mode**: Uses optimized rendering that only updates changed pixels
- **Render Distance**: Higher values increase calculation time exponentially
- **Resolution**: Character mode resolution affects ASCII detail; color mode uses fixed 192×54
- **Spherical Projection**: May be more computationally intensive

### Troubleshooting

#### Common Issues
1. **No color display**: Ensure terminal supports ANSI escape codes
2. **Missing textures**: Verify texture files exist in correct format
3. **Low FPS**: Reduce resolution or render distance
4. **Save/load errors**: Check directory permissions

#### Texture Requirements
- Must be 16×16 grids of RGB values
- Six files per texture pack (one per cube face)
- Naming convention: `[texture_name][1-6].txt`

### Development Notes
- Uses ray marching algorithm for voxel rendering
- Implements custom 3D vector math library
- Optimized for real-time performance
- Supports modular texture system

### Credits
- 3D rendering engine with custom mathematics
- Real-time interactive voxel world
- Dual rendering system for compatibility

---

## License & Attribution
These programs are provided for educational and demonstration purposes. They demonstrate 3D rendering techniques and voxel graphics implementation.

## Support
For issues or questions, please check:
1. Ensure all required directories exist
2. Verify texture files are in correct format
3. Check terminal compatibility for color mode

Enjoy exploring the voxel worlds!
