# best-fn-base

Windows C++20 user-mode overlay base for experimenting with DirectX 11, ImGui rendering, process-window tracking, SDK math helpers, and driver-backed memory access around a Fortnite client.

This repository is intended as a private development and learning base. Use it only in environments where you have permission to test.

## Overview

The application waits for `FortniteClient-Win64-Shipping.exe`, resolves the game window, initializes the configured driver communication layer, and starts a transparent DirectX 11 overlay loop. The render loop updates a small game-state cache, draws ImGui menu controls, and renders enabled visual primitives.

Current feature areas include:

- Target process/window detection.
- Driver communication wrappers for physical memory reads/writes.
- DirectX 11 + ImGui overlay initialization.
- Camera projection, matrix/vector math, bone lookup, and visibility helpers.
- Configurable menu state for FOV display and basic visual options.
- Box, line, and distance drawing helpers.

## Project Layout

```text
.
|-- main.cpp                 Entry point and startup flow
|-- includes.h               Shared include hub
|-- core/
|   |-- loop.h               Game-state cache update logic
|   `-- visuals/             Visual rendering and draw helpers
|-- driver/
|   |-- comms.h              User-mode driver communication wrapper
|   `-- kernel/drv.h         Driver interface declarations
|-- overlay/
|   |-- overlay.h            Window, D3D11, ImGui, and render loop setup
|   |-- menu.h               ImGui menu controls
|   `-- imgui/               Vendored ImGui sources/backends
|-- sdk/
|   |-- sdk.h                Camera, world-to-screen, bone, visibility helpers
|   |-- offsets.h            Target offsets
|   `-- math/                Vector and matrix types
|-- settings/
|   `-- settings.h           Runtime menu/visual settings
|-- um.sln                   Visual Studio solution
`-- um.vcxproj               Visual Studio C++ project
```

## Requirements

- Windows.
- Visual Studio with the Desktop development with C++ workload.
- Windows 10 SDK.
- Platform toolset `v145`.
- C++20 language support.
- A compatible driver/comms layer matching the interfaces in `driver/`.

DirectX 11 and DWM libraries are linked by the project/source pragmas.

## Build

Open `um.sln` in Visual Studio, select `x64`, choose `Release` or `Debug`, and build the `um` project.

Command-line build example:

```powershell
msbuild um.sln /p:Configuration=Release /p:Platform=x64
```

## Runtime Notes

- The program waits for `FortniteClient-Win64-Shipping.exe` before continuing startup.
- The overlay code looks for an existing `Discord Overlay` window and uses it as the render surface.
- Offsets live in `sdk/offsets.h` and may need updates when the target changes.
- Generated Visual Studio files, local agent folders, and build outputs are intentionally ignored by `.gitignore`.

