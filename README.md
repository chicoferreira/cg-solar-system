# CG Solar System

![wallpaper](https://github.com/user-attachments/assets/e7a302cc-88eb-442a-b661-8cada1ecb2e6)

This repository contains the source code for the practical assignment of Computer Graphics course at Universidade do Minho.

The assignment had the requirement to implement an engine using **OpenGL 2** capable of loading and rendering arbitrary worlds defined in a custom XML format, ultimately creating a complete solar system with planets, comets and orbit rotations.

## Main features

### Model primitives

The generator program, which was also part of the assignment, allows the creation of 3D models to be used in scene files loaded by the engine. The engine supports both the custom .3d format and Wavefront .obj files. The program outputs vertex positions, normals, texture coordinates, and vertex indices to a text file in the custom .3d format. This generator can also create complete scenes like the solar system.

```bash
Usage: generator <command> <args> <output file>
Commands:
        generator plane <length> <divisions> <output file>
        generator sphere <radius> <slices> <stacks> <output file>
        generator cone <radius> <height> <slices> <stacks> <output file>
        generator box <length> <divisions> <output file>
        generator cylinder <radius> <height> <slices> <output file>
        generator patch <patch_file> <tesselation> <output file>
        generator solar-system <sun size scale factor> <planet distance scale factor> <scene scale factor> <number of asteroids> <output file>
```

### [ImGui](https://github.com/ocornut/imgui)

- Toggle settings: lights, v-sync, backface culling, wireframe mode, MSAA, frustum culling, render axes, display normals, show AABB for frustum culling, show temporal translation path, and display light models.
- Control simulation: pause, resume, and adjust speed.
- Camera configuration (also possible with WASD + Space + Ctrl): modify position, FOV, acceleration, friction, and more.
- Manage lights: add, remove and list all lights in the scene.
- Edit world models: view and modify material properties and transformations of each model in the scene graph.
- Display vertex information of a model in a table format.

![ImGUI](https://github.com/user-attachments/assets/489a9955-2cde-442f-a90d-e0612f0bc6cd)

### Frustum Culling

Implemented frustum culling using AABB intersection with view planes. The AABB dynamically updates with object scaling and rotation. No space partition optimization is currently implemented.

https://github.com/user-attachments/assets/733ba257-9e6d-4684-9ec2-2cbf7a4e4390

### Smooth Camera Transitions (FPV & TPV)

First-person and third-person camera views with smooth acceleration and deceleration for nice transitions between motion states with many parameters configurable.

https://github.com/user-attachments/assets/3f669ac1-d1ef-470f-b943-1f87927569be

### Solar System

![Demo](https://github.com/user-attachments/assets/bb9cae36-967a-4d15-a5a5-78d1f9c180e6)

### Others

More features can be found in the [four project reports](./reports). While they are written in Portuguese, they include additional images that may still be useful to non-Portuguese readers.

## Compiling

Prerequisites:

- [vcpkg](https://vcpkg.io/en/getting-started)
- [CMake](https://cmake.org/download/)
- [MSBuild](https://visualstudio.microsoft.com/downloads/) on Windows or GCC/CLang on Linux and macOS

#### With CLion

1. Open the project in CLion.
2. If prompted to install the toolchain, click `Install`.
3. Navigate to `File` -> `Settings` -> `Build, Execution, Deployment` -> `CMake` and add the following flag to `CMake options`:
   ```bash
   -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
   ```
   You can find this path by running `vcpkg integrate install` and looking at the output.
4. Reload the CMake project.

#### With no IDE

```bash
$ cd cg-solar-system
$ cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
$ cmake --build build
$ .\build\engine\Debug\cg-solar-system.exe <scene> # For running the engine
$ .\build\generator\Debug\cg-generator.exe <args> # For running the generator
```

You can find the `[path to vcpkg]` by running `vcpkg integrate install` and looking at the output.
