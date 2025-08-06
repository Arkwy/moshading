# Moshading

**Moshading** (name subject to change) is a cross-platform visual effects mixer written in modern C++ (C++26), leveraging **WebGPU** and **Dear ImGui**. It allows users to load and display images or videos in customizable frames, and apply real-time visual effects through user-editable **WGSL shaders**.

Currently, Moshading supports **Linux (Wayland)** and **WebAssembly/Web** builds. More platforms are planned in future development.



## Website

You can try the web version here [arkwy.github.io/moshading](https://arkwy.github.io/moshading).

WebGPU must be supported and enabled in your browser. Some hardware may not support it.

Tested with [google chrome](https://www.google.fr/chrome/) (requires enabling `chrome://flags/#enable-unsafe-webgpu`) and [firefox nightly](https://www.firefox.com/channel/desktop/).



## Features

- Load and display images or videos
- Apply customizable visual effects via WGSL shaders
- Real-time rendering using WebGPU
- GUI powered by Dear ImGui



## Build Instructions

Moshading uses **Meson** as its build system. A `flake.nix` is also provided for Nix-based development environments and builds.

### Dependencies

Native builds:
- `glfw3`
- `gl`
- `glew`
- `wgpu-native`

Web build:
- `emscripten`

Common:
- `clang`
- `meson`
- `ninja`
- `python3` (for asset embedding scripts)



### Build on Linux (Wayland)

```sh
meson setup builddir [--buildtype=release]
meson compile -C builddir
./builddir/moshading
````

Run with:
```sh
./builddir/moshading
```

### Build for Web (WASM)

```sh
meson  setup builds/web --cross-file wasm.cross [--buildtype=release]
meson compile -C builddir
```

Run with:
```sh
cp builddir/moshading.js builddir/moshading.wasm build/
python -m http.server
```

Go to [localhost:8000](http://127.0.0.1:8000) with a web browser with WebGPU enabled to access the app.



### Nix Flake (dev env & build)

If you're using **Nix**:

```sh
# Enter development environment
nix develop

# Or build using nix
nix build
```



##  Roadmap / TODO

* [x] Linux Wayland support
* [x] WebAssembly (browser) support
* [ ] **Video import**
* [ ] **Export to image/video**
* [ ] **X11 support**
* [ ] **Windows support**
* [ ] **macOS support**
* [ ] **Node-based visual editor**
* [ ] **Project save/load system**
* [ ] **Enhance shader effects library**
* [ ] **Support for user-written shaders**
* [ ] **Shader parser for automatic parameter creation**
* [ ] **Audio-reactive effects ?**



## Project Structure

```
src/                  -> Main application source
shaders/              -> WGSL shaders
imgui/, stb/, ...     -> Third-party libraries
utils/embed_*.py      -> Python scripts for asset embedding
meson.build           -> Meson build configuration
flake.nix             -> Nix flake (dev env + build)
```



## License

MIT License

## Acknowledgements

- [WebGPU-Cpp](https://github.com/eliemichel/WebGPU-Cpp) by Elie Michel for C++ WebGPU headers.
- [Dear ImGui](https://github.com/ocornut/imgui) for the GUI.
- [Portable File Dialogs](https://github.com/samhocevar/portable-file-dialogs) for file selection dialogs.
- [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders) for icon integration.
- [stb](https://github.com/nothings/stb) for image loading.

---

Moshading is in early development — feedback and contributions are welcome!
