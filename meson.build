project('moshading', 'cpp', default_options : ['cpp_std=c++23'])

glfw_dep = dependency('glfw3')
imgui_dep = dependency('imgui')
gl_dep = dependency('gl', required: true)
glew_dep = dependency('glew', required: true)

files = [
  'src/main.cpp',
  'src/shader.cpp',
  'src/shader_parameter.cpp',
]


executable('moshading',
  files,
  dependencies: [glfw_dep, gl_dep, glew_dep, imgui_dep],
  cpp_args: ['-g', '-O3'],
)
