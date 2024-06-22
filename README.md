# CG Solar System

The CG Solar System project is a 3D engine built using OpenGL 2.0, showcasing a detailed model of the solar system. It features advanced functionalities such as ImGui integration for interactive exploration, temporal transformations, and efficient rendering techniques like VBOs with indices.

![Solar System Overview](images/solar_system_overview.png)
*An overview of the solar system visualization.*

![ImGui Integration](images/imgui_integration.png)
*ImGui integration for interactive exploration.*

## Project Description

The CG Solar System project aims to create a realistic and interactive 3D representation of the solar system. It utilizes OpenGL for rendering and ImGui for an interactive interface, allowing users to explore the solar system in detail. The project highlights include:

- Detailed models of planets and moons with accurate scaling and positioning.
- Temporal transformations to simulate planetary orbits and rotations.
- Integration with ImGui to provide a user-friendly interface for navigation and exploration.
- Use of Vertex Buffer Objects (VBOs) with indices for efficient rendering.

## Setup and Build Instructions

### Prerequisites

- [vcpkg](https://vcpkg.io/en/getting-started.html) for managing dependencies
- [CMake](https://cmake.org/download/) for building the project
- A C++ compiler (MSVC for Windows, GCC for Linux, Clang for MacOS)

### Windows

1. Clone the repository and initialize submodules:
   ```bash
   git clone https://github.com/chicoferreira/cg-solar-system.git
   cd cg-solar-system
   ```
2. Install dependencies using vcpkg:
   ```bash
   vcpkg install
   ```
3. Generate build files with CMake:
   ```bash
   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
   ```
4. Build the project:
   ```bash
   cmake --build build --config Release
   ```

### Linux and MacOS

The steps are similar to Windows, but ensure you have the appropriate compiler installed for your platform. Use your system's package manager to install dependencies if necessary.

## Main Functionalities

### ImGui Integration

The project integrates ImGui to provide a user-friendly interface for exploring the solar system. Users can interact with various elements, adjust settings, and navigate through the solar system in real-time.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
