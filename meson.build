project('moshading', ['cpp', 'c'])

cpp = meson.get_compiler('cpp')
is_wasm = cpp.get_id() == 'emscripten'

buildtype = get_option('buildtype')
is_debug = buildtype == 'debug'

# Shader embedding
embed_shaders = custom_target(
    'embed_shaders',
    output: ['shaders_code.cpp', 'shaders_code.hpp'],
    command: ['python', meson.current_source_dir() / 'utils/embed_shaders.py', '@OUTPUT@', meson.current_source_dir() / 'shaders/'],
    build_always_stale: not is_debug,
)
embed_shaders_dep = declare_dependency(sources: embed_shaders)

# Icon embedding
embed_icons = custom_target(
    'embed_icons',
    output: ['icons.cpp', 'icons.hpp'],
    command: ['python', meson.current_source_dir() / 'utils/embed_icons.py', meson.current_source_dir() / 'Font-Awesome/webfonts/fa-solid-900.ttf', '@OUTPUT@'] + (
      is_debug ? [] : ['--subset', '']
    ),
    build_always_stale: not is_debug,
    
)
embed_icons_dep = declare_dependency(sources: embed_icons)

# Submodules
imgui_dir = 'imgui'
imgui_files = [
    imgui_dir / 'imgui.cpp',
    imgui_dir / 'imgui_draw.cpp',
    imgui_dir / 'imgui_widgets.cpp',
    imgui_dir / 'imgui_tables.cpp',
    imgui_dir / 'backends/imgui_impl_glfw.cpp',
    imgui_dir / 'backends/imgui_impl_wgpu.cpp',
]
pfd_dir = 'portable-file-dialogs'
icon_headers_dir = 'IconFontCppHeaders'
stb_dir = 'stb'
stb_files = [stb_dir / 'stb_image.cpp']

# App source files
moshading_files = [
  'src/main.cpp',
  'src/context/gpu.cpp',
  'src/renderer.cpp',
  'src/app.cpp',
  'src/file_loader.cpp',
  'src/shader/manager.cpp',
  'src/shader/parameter.cpp',
  embed_shaders[0],
  embed_icons[0],
]

files = moshading_files + imgui_files + stb_files

# Base warnings
warning_args = ['-Wall', '-Wextra']

# Debug and Release compile flags
debug_args = [
  '-DDEBUG',
  '-O1',
  '-g',
  '-fno-inline',
  '-fsanitize=address',
  '-fsanitize=undefined',
  '-fsanitize=leak',
]

release_args = [
  '-O3',
]

compile_args = warning_args + (is_debug ? debug_args : release_args)
link_args = is_debug ? ['-fsanitize=address', '-fsanitize=undefined', '-fsanitize=leak'] : []

# Platform-specific logic
if is_wasm
  files += files('src/renderer_web.cpp')

  emscripten_flags = [
      '-sUSE_GLFW=3',
      '-sUSE_WEBGPU=1',
      '-sEXPORT_ALL=1',
      '-sASYNCIFY',
      '-sWASM=1',
      '-sALLOW_MEMORY_GROWTH=1',
      '-sNO_EXIT_RUNTIME=0',
      '-sASSERTIONS=1',
      '-sDISABLE_EXCEPTION_CATCHING=0',
      '-sEXPORTED_RUNTIME_METHODS=ccall',
  ]

  executable(
      'moshading',
      files,
      include_directories: [
          include_directories(imgui_dir),
          include_directories(pfd_dir),
          include_directories(icon_headers_dir),
          include_directories(stb_dir),
          include_directories('webgpu-cpp/emscripten'),
      ],
      dependencies: [
          embed_shaders_dep,
          embed_icons_dep,
      ],
      cpp_args: ['-std=c++26'] + compile_args + (is_debug ? ['-fexceptions'] : []),
      link_args: emscripten_flags + link_args,
      install: false,
  )
else
  glfw_dep = dependency('glfw3')
  glew_dep = dependency('glew')
  wgpu_dep = dependency('wgpu-native')
  # wayland_dep = dependency('wayland-client')

  files += files('src/renderer_native.cpp')

  executable(
      'moshading',
      files,
      include_directories: [
          include_directories(imgui_dir),
          include_directories(pfd_dir),
          include_directories(icon_headers_dir),
          include_directories(stb_dir),
          include_directories('webgpu-cpp/wgpu-native'),
      ],
      dependencies: [
          embed_shaders_dep,
          embed_icons_dep,
          glfw_dep,
          glew_dep,
          wgpu_dep,
          # wayland_dep,
      ],
      cpp_args: ['-DIMGUI_IMPL_WEBGPU_BACKEND_WGPU', '-DWEBGPU_BACKEND_WGPU', '-std=c++26'] + compile_args,
      link_args: link_args,
      install: false,
  )
endif
