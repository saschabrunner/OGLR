project('opengl_renderer', 'c', 'cpp',  default_options : ['c_std=c11', 'cpp_std=c++14'])

# copy assets
datadir = join_paths(get_option('datadir'), meson.project_name())
install_subdir('shaders', install_dir: datadir)
install_subdir('textures', install_dir: datadir)

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
    'DataDirHelper.cxx',
    'FpsCamera.cxx',
    'Mesh.cxx',
    'Shader.cxx',
    'glad/src/glad.c'
]

incdirs = include_directories([
    'glad/include'
])

# compile the binary
executable('opengl_renderer', src, dependencies: deps, include_directories: incdirs, install: true)