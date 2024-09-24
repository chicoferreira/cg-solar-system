# CG Solar System
![Main picture](https://github.com/user-attachments/assets/ddac9b11-5934-4171-b1f3-2a48efcf0f3f)

Repository containing the source code for the practical assignment of Computer Graphics course at Universidade do Minho.

The assignment had the requirement to implement an engine using **OpenGL 2** that can load arbitrary worlds from a custom XML format and render it in real time, ultimately leading to render a complete solar system with planets, comets and rotations.

## Main features

### Model primitives

Using the generator program, it's possible to generate 3d models that can be used in a scene file loaded in the engine program (_Wavefront .obj_ files are also supported). 
This program outputs to a text file containing vertex position, normals, texture coordinates and vertex indices of the generated model.

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

- Enable and disable: lights, _v-sync_, backface culling, wireframe mode, MSSA, frustum culling, rendering axis, showing normals, showing AABB (for frustum culling), showing temporal translation path and light models.
- Pause and resume simulation time and also speed up or slow down the speed.
- Setup camera (also possible with WASD + Space + Control): change position, fov, acceleration, friction speed, etc.
- Add and remove lights and list them.
- List every model in the scene graph and edit their material properties and transforms.
- Show model vertex information in a table format.

![ImGUI](https://github.com/user-attachments/assets/489a9955-2cde-442f-a90d-e0612f0bc6cd)

### Frustum Culling

https://github.com/user-attachments/assets/733ba257-9e6d-4684-9ec2-2cbf7a4e4390

### FPV with smooth acceleration and deacceleration

### Others

## Compiling

Prerequisites:

- [vcpkg](https://vcpkg.io/en/getting-started)
- [CMake](https://cmake.org/download/)
- [MSBuild](https://visualstudio.microsoft.com/downloads/) on Windows or GCC/CLang on Linux and MacOS

#### With CLion

1. Open the project
2. CLion will ask to install the toolchain, click on `Install`
3. Go to `File` -> `Settings` -> `Build, Execution, Deployment` -> `CMake` and add the
   flag `-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake` to `CMake options`.
   You can find this path by running `vcpkg integrate install` and looking at the output.
4. Reload the CMake project

#### With no IDE

```bash
$ cd cg-solar-system
$ cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake
$ cmake --build build
$ .\build\engine\Debug\cg-solar-system.exe <scene> # For running the engine
$ .\build\generator\Debug\cg-generator.exe <args> # For running the generator
```

You can find the `[path to vcpkg]` by running `vcpkg integrate install` and looking at the output.
