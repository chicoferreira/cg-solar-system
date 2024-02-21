## Compiling

### Windows

Prerequisites:

- [vcpkg](https://vcpkg.io/en/getting-started)
- [CMake](https://cmake.org/download/)
- [MSBuild](https://visualstudio.microsoft.com/downloads/)

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
$ ./build/Debug/cg-solar-system.exe
```

You can find the vcpkg path by running `vcpkg integrate install` and looking at the output.