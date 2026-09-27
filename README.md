# hierstate

A small C++ library for hierarchical state machines.

## Build and Test

Requirements:

- CMake 3.28 or newer
- A C++20 toolchain:
  - macOS: Xcode or Command Line Tools
  - Windows: Visual Studio 2022 or Build Tools 2022 with the Desktop development with C++ workload

On macOS:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

On Windows, use Developer PowerShell for Visual Studio 2022:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

## Use from CMake

Add hierstate to your project as a subdirectory, then link its target:

```cmake
add_subdirectory(external/hierstate)
target_link_libraries(my_game PRIVATE hierstate)
```

To use an installed copy, install it to a prefix and locate the package from a separate project:

```sh
cmake --install build --prefix /path/to/hierstate-install
```

```cmake
find_package(hierstate CONFIG REQUIRED)
target_link_libraries(my_game PRIVATE hierstate)
```

Pass `-DCMAKE_PREFIX_PATH=/path/to/hierstate-install` when configuring the consuming project if the prefix is outside CMake's default search paths.

## How to Use

Include `<hierstate.h>` to use `hierstate::state` and `hierstate::group`.

See [examples/sequence.cpp](examples/sequence.cpp) for a runnable hierarchy and the use of `current()`, `get()`, `enter()`, and `exit()`.

## API Contracts

- Each group's `Id` must be an enum type. The group owns one child, and `start(id)` selects it only once. The derived group's `create(id)` must create a child for every supported value and reject unsupported values.
- `request(id)` keeps the first pending request. `update()` finishes the current child's update before constructing the replacement and changing children. If construction fails, the current child stays active and the request stays pending.
- `current()` returns the selected child ID. `get()` returns a non-owning, read-only pointer to that child, or `nullptr` before `start()`. A transition or group destruction invalidates that pointer; acquire it again after `update()`.
- `start(id)` does not call `enter()`. `update()` enters the initial child if it has not been entered yet. Call `exit()` on the root group before destroying it when exit actions are needed; destruction alone does not call exit hooks. A group's `exit()` propagates to its child before returning.
- `enter()` and `exit()` must not throw. Put potentially failing setup in `create(id)` or a state constructor. `start(id)` and `update()` may report errors with exceptions.
