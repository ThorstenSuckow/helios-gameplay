# helios::gameplay

Gameplay related systems for the helios engine modules.

## Overview

`helios::gameplay` connects engine-level gameplay resources to game logic.

## Features

- Spawning module for spawn placement and initialization of game objects and particles.

## Module surface

## Usage

### C++ module

```cpp
import helios.gameplay;
```

### Backend architecture

### CMake

Build and install:

```bash
cmake -S . -B build -DHELIOS_GAMEPLAY_ENABLE_PACKAGE=ON -DCMAKE_INSTALL_PREFIX="$PWD/install"
cmake --build build
cmake --install build
```

Consume from another project:

```cmake
find_package(helios-engine CONFIG REQUIRED)
find_package(helios-gameplay CONFIG REQUIRED)
target_link_libraries(your_target PRIVATE helios::gameplay)
```

Configure a consumer against an installed prefix:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="/path/to/helios-prefix"
cmake --build build
```

## Development

Run the regular CMake build from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

## Related repositories

- [`helios-ecs`](https://github.com/thorstensuckow/helios-ecs)
- [`helios-engine`](https://github.com/thorstensuckow/helios-engine)
- [`helios-math`](https://github.com/thorstensuckow/helios-math)
- [`helios-glfw`](https://github.com/thorstensuckow/helios-glfw)
- [`helios-physics`](https://github.com/thorstensuckow/helios-physics)