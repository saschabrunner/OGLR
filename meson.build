project('opengl_renderer', 'c', 'cpp',  default_options : ['c_std=c11', 'cpp_std=c++14'])

# copy assets
datadir = join_paths(get_option('datadir'), meson.project_name())
install_subdir('data', install_dir: datadir, strip_directory : true)

# execute buildfile for the executable
subdir('src')