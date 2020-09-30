# generate config header
config = configuration_data()
config.set('datadir', datadir)
config.set('project_name', meson.project_name())
configure_file(
    input: 'config.h.in',
    output: 'config.h',
    configuration: config
)

cpp = meson.get_compiler('cpp')
# we need to find the dl library for runtime dynamic linking
libdl = cpp.find_library('dl')

# the c++17 filesystem library still needs to be manually linked in many cases
# due to incompatibilities of an old glibmm from MXE with C++17, we use boost::filesystem for now
#cppfilesystem = cpp.find_library('stdc++fs')

deps = [
    libdl,
    dependency('glfw3'),
    dependency('assimp'),
    dependency('gl'),
    dependency('glibmm-2.4'),
    dependency('boost', modules : ['system', 'filesystem'])
]

src = [
    'main.cxx',
    'Camera.cxx',
    'DirectoryHelper.cxx',
    'FpsCamera.cxx',
    'Mesh.cxx',
    'Shader.cxx',
    'lib/glad/src/glad.c',
    'lib/imgui/imgui.cpp',
    'lib/imgui/imgui_demo.cpp',
    'lib/imgui/imgui_draw.cpp',
    'lib/imgui/imgui_widgets.cpp',
    'lib/imgui/imgui_impl_glfw.cpp',
    'lib/imgui/imgui_impl_opengl3.cpp'
]

incdirs = include_directories([
    'lib/glad/include'
])

# compile the binary
executable('opengl_renderer', src, dependencies: deps, include_directories: incdirs, install: true)