project('moshading', 'cpp', default_options : ['cpp_std=c++23'])

wgpu_dep = declare_dependency()

glfw_dep = dependency('glfw3')
gl_dep = dependency('gl', required: true)
glew_dep = dependency('glew', required: true)

imgui_files = [
  'imgui/imgui.cpp',
  'imgui/imgui_draw.cpp',
  'imgui/imgui_widgets.cpp',
  'imgui/imgui_tables.cpp',
  'imgui/backends/imgui_impl_glfw.cpp',
  'imgui/backends/imgui_impl_opengl3.cpp',
]

moshading_files = [
  'src/main.cpp',
  'src/shader.cpp',
  'src/shader_parameter.cpp',
]

files = moshading_files + imgui_files

executable(
  'moshading',
  files,
  include_directories: ['imgui', 'imgui/backends'],
  dependencies: [glfw_dep, gl_dep, glew_dep, wgpu_dep],
  cpp_args: ['-g', '-O3'],
)
