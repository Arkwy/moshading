project('moshading', ['cpp', 'c'], default_options : ['cpp_std=c++26'])

cpp = meson.get_compiler('cpp')
is_wasm = cpp.get_id() == 'emscripten'


# embed shaders in code
embed_shaders = custom_target(
    'embed_shaders',
    output: ['shaders_code.cpp', 'shaders_code.hpp'],
    command: ['python', meson.current_source_dir() / 'utils/embed_shaders.py', '@OUTPUT@', meson.current_source_dir() / 'shaders/'],
    build_always_stale: true,
)

embed_shaders_dep = declare_dependency(sources: embed_shaders)

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

# stb
stb_dir = 'stb'
stb_files = [ stb_dir / 'stb_image.cpp' ]

# tinyfd
tinyfd_dir = 'tinyfiledialogs'
tinyfd_files = [ tinyfd_dir / 'tinyfiledialogs.c' ]

# Source files
moshading_files = [
  'src/main.cpp',
  'src/gpu_context.cpp',
  'src/renderer.cpp',
  'src/app.cpp',
  'src/file_loading.cpp',
  'src/shader/manager.cpp',
  'src/shader/parameter.cpp',
  # 'src/shader/shaders/circle.cpp',
  embed_shaders[0],
]

files = moshading_files + imgui_files + stb_files

if is_wasm

    files += 'src/renderer_web.cpp'

    emscripten_flags = [
        '-sUSE_GLFW=3',
        '-sUSE_WEBGPU=1',
        '-sEXPORT_ALL=1', # TODO remove in release build
        # '-sASYNCIFY',
        '-sWASM=1',
        '-sALLOW_MEMORY_GROWTH=1',
        '-sNO_EXIT_RUNTIME=0',
        '-sASSERTIONS=1',
        '-sDISABLE_EXCEPTION_CATCHING=0',
        '-sNO_FILESYSTEM=1',
        # '--bind', # optional if using embind
    ]

    executable(
        'moshading',
        files,
        include_directories: [
            include_directories(imgui_dir),
            include_directories(stb_dir),
            include_directories('emscripten'),
        ],
        dependencies: [],
        cpp_args: ['-g', '-O3', '-fexceptions'],
        link_args: emscripten_flags,
        install: false,
    )
else
    glfw_dep = dependency('glfw3')
    gl_dep = dependency('gl', required: true)
    glew_dep = dependency('glew', required: true)
    wgpu_dep = dependency('wgpu-native', required: true)

    # subproject('wgpu-native') # Make sure this exists or skip if not using native WebGPU

    files += 'src/renderer_native.cpp'

    executable(
        'moshading',
        files + tinyfd_files,
        include_directories: [
            include_directories(imgui_dir),
            include_directories(stb_dir),
            include_directories('wgpu-native'),
        ],
        dependencies: [
            embed_shaders_dep,
            glfw_dep,
            gl_dep,
            glew_dep,
            wgpu_dep,
        ],
        link_args: ['-lwgpu_native'],
        cpp_args: ['-g', '-O3', '-DIMGUI_IMPL_WEBGPU_BACKEND_WGPU', '-DWEBGPU_BACKEND_WGPU'],
        install: false,
    )
endif
