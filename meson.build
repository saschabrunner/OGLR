project('opengl_renderer', 'c', 'cpp')

cpp = meson.get_compiler('cpp')
libdl = cpp.find_library('dl')

deps = [
    dependency('glfw3'),
    libdl,
    dependency('assimp'),
    dependency('gl')
]

src = [
    'main.cxx',
    'Camera.cxx',
    'FpsCamera.cxx',
    'Mesh.cxx',
    'Shader.cxx',
    'glad/src/glad.c'
]

incdirs = include_directories([
    'glad/include'
])

executable('opengl_renderer', src, dependencies: deps, include_directories: incdirs)