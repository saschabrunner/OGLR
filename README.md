# OpenGL Renderer

## Description
A very simple OpenGL renderer.
It is created based on the exercises in the OpenGL tutorial on https://learnopengl.com/ by Joey de Vries.

## Building

### Dependencies
* C++14
* [GLFW3](https://www.glfw.org/)
* [Assimp](https://www.assimp.org/)
* [Glibmm](https://developer.gnome.org/glibmm/stable/)
* [Boost](https://www.boost.org/) (system and filesystem modules)
* Anything OpenGL 3.3 or larger

### Build
You need the above dependencies and Meson to build.

Example steps on a Linux system:
```
meson build --prefix /preferred/install/location
cd build
ninja install
```

This will build the application and copy all its data to /preferred/install/location.
Supplying a prefix to meson (`--prefix /preferred/install/location`) is optional. If left out, the application will be installed to your system in the respective directories (usually /usr/bin for the binary and /usr/share for the assets).

### Windows support
Windows support is given through using MXE to cross-compile into a Windows binary.
Other methods (like using MSYS on Windows) may be supported but have not been tested yet. They might be explored in the future.

To use MXE, follow the guide at https://mxe.cc/#tutorial
Then enable a more recent version of GCC and the Meson wrapper plugin and build the required packages:

```
make \
    MXE_TARGETS='x86_64-w64-mingw32.static' \
    MXE_PLUGIN_DIRS='plugins/gcc10 plugins/meson-wrapper' \
    gcc \
    assimp \
    boost \
    glfw3 \
    glibmm \
    meson-wrapper \
    --jobs 4 \
    JOBS=2
```

`MXE_TARGETS='x86_64-w64-mingw32.static'` is used to create a 64-bit toolchain that creates a statically linked binary
`MXE_PLUGIN_DIRS='plugins/gcc10 plugins/meson-wrapper'` enables a newer version of GCC and enables the meson-wrapper script
`--jobs 4` enables up to four packages being built in parallel
`JOBS=2` to compile up to two source files per project in parallel

After that completes, a new build target for a Windows binary can be created inside the project directory as follows:
```
x86_64-w64-mingw32.static-meson build --prefix /preferred/install/location
cd build
ninja install
```