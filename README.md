# DVD Animation

Bouncing DVD logo animation built with **C++23**, **SDL3**, and **OpenGL**.

![screenshot](assets/dvd.png)

## Features

- Smooth bouncing animation with delta-time based movement
- Color change on edge collision (rainbow palette)
- OpenGL rendering with texture mapping
- RAII wrappers for `SDL_Window`, `SDL_GLContext`, and `Texture`
- C++20 modules via `zethcxx.stx`

## Requirements

- C++23 compiler (Clang 19+)
- xmake 2.8.0+
- SDL3, OpenGL, stb

## Build & Run

```bash
xmake
xmake run
```

The window is resizable; the logo adapts to the new dimensions.

## Project Structure

```
├── app/
│   └── main.cpp          # Entry point and render loop
├── assets/
│   └── dvd.png           # DVD logo texture
├── xmake/
│   ├── cfg_flags.lua     # Compiler flag configuration
│   └── cfg_triple.lua    # Target triple detection
└── xmake.lua             # Build configuration
```
