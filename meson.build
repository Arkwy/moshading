project('moshading', 'cpp', default_options : ['cpp_std=c++23'])

# Dependencies
glfw_dep = dependency('glfw3')
gl_dep = dependency('gl', required: true)
glew_dep = dependency('glew', required: true)

# Declare the wgpu-native subproject
wgpu_proj = subproject('wgpu-native')

# ImGui files
imgui_dir = 'imgui'
imgui_files = [
  imgui_dir / 'imgui.cpp',
  imgui_dir / 'imgui_draw.cpp',
  imgui_dir / 'imgui_widgets.cpp',
  imgui_dir / 'imgui_tables.cpp',
  imgui_dir / 'backends/imgui_impl_glfw.cpp',
  imgui_dir / 'backends/imgui_impl_wgpu.cpp',
]

# Source files
moshading_files = [
  'src/main.cpp',
  'src/shader.cpp',
  'src/shader_parameter.cpp',
]

files = moshading_files + imgui_files

executable(
  'moshading',
  files,
  include_directories: [
    imgui_dir,
    imgui_dir / 'backends',
  ],
  dependencies: [
    glfw_dep,
    gl_dep,
    glew_dep,
  ],
  link_args: ['-lwgpu_native'],
  cpp_args: ['-g', '-O3', '-DIMGUI_IMPL_WEBGPU_BACKEND_WGPU'],
)
