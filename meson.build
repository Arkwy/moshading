project('moshading', ['cpp', 'c']) #, default_options : ['cpp_std=c++26'])

cpp = meson.get_compiler('cpp')
is_wasm = cpp.get_id() == 'emscripten'


# embed shaders in code
embed_shaders = custom_target(
    'embed_shaders',
    output: ['shaders_code.cpp', 'shaders_code.hpp'],
    command: ['python', meson.current_source_dir() / 'utils/embed_shaders.py', '@OUTPUT@', meson.current_source_dir() / 'shaders/'],
    build_always_stale: true, # force rerun at each compilation
)
embed_shaders_dep = declare_dependency(sources: embed_shaders)


# embed icons in code
embed_icons = custom_target(
    'embed_icons',
    output: ['icons.cpp', 'icons.hpp'],
    command: ['python', meson.current_source_dir() / 'utils/embed_icons.py', meson.current_source_dir() / 'Font-Awesome/webfonts/fa-solid-900.ttf', '@OUTPUT@'],
    # build_always_stale: true, # force rerun at each compilation
)
embed_icons_dep = declare_dependency(sources: embed_icons)


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


# Portable file dialogs
pfd_dir = 'portable-file-dialogs'

# Icon font cpp headers
icon_headers_dir = 'IconFontCppHeaders'

# stb
stb_dir = 'stb'
stb_files = [ stb_dir / 'stb_image.cpp' ]

# Source files
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
        # '-sNO_FILESYSTEM=1',
        # '--bind', # optional if using embind
    ]

    executable(
        'moshading',
        files,
        include_directories: [
            include_directories(imgui_dir),
            include_directories(pfd_dir),
            include_directories(icon_headers_dir),
            include_directories(stb_dir),
            include_directories('emscripten'),
        ],
        dependencies: [
            embed_shaders_dep,
            embed_icons_dep,
        ],
        cpp_args: ['-g', '-O3', '-fexceptions', '-std=c++26'],
        link_args: emscripten_flags,
        install: false,
    )
else
    glfw_dep = dependency('glfw3')
    gl_dep = dependency('gl', required: true)
    glew_dep = dependency('glew', required: true)
    wgpu_dep = dependency('wgpu-native', required: true)

    files += 'src/renderer_native.cpp'

    executable(
        'moshading',
        files,
        include_directories: [
            include_directories(imgui_dir),
            include_directories(pfd_dir),
            include_directories(icon_headers_dir),
            include_directories(stb_dir),
            include_directories('wgpu-native'),
        ],
        dependencies: [
            embed_shaders_dep,
            embed_icons_dep,
            glfw_dep,
            gl_dep,
            glew_dep,
            wgpu_dep,
        ],
        link_args: ['-lwgpu_native'],
        cpp_args: ['-g', '-O0', '-DIMGUI_IMPL_WEBGPU_BACKEND_WGPU', '-DWEBGPU_BACKEND_WGPU', '-ftemplate-backtrace-limit=0', '-std=c++26'],
        install: false,
    )
endif
