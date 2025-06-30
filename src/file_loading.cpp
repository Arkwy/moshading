#include <iostream>
#include <vector>
#include "src/shader/shader.hpp"
#include "file_loading.hpp"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/bind.h>

// Callback from JS with file data
extern "C" void on_file_loaded(uintptr_t dataPtr, int size) { // , void* manager_ptr) {
    // ShaderManager manager = reinterpret_cast<uint8_t*>(manager_ptr) 

    const uint8_t* data = reinterpret_cast<uint8_t*>(dataPtr);
    std::vector<uint8_t> file_data(data, data + size);

    std::cout << "File loaded in C++, size: " << size << " bytes" << std::endl;

    // Example: First 10 bytes
    for (int i = 0; i < std::min(size, 10); i++) {
        std::cout << std::hex << (int)file_data[i] << " ";
    }
    std::cout << std::endl;
}

// Trigger JS file dialog
EM_JS(void, open_file_dialog, (), {
  const input = document.createElement('input');
  input.type = 'file';
  input.onchange = e => {
    const file = e.target.files[0];
    const reader = new FileReader();

    reader.onload = function(evt) {
      const arrayBuffer = evt.target.result;
      const byteArray = new Uint8Array(arrayBuffer);

      // Allocate memory in the WASM heap
      const ptr = Module._malloc(byteArray.length);
      Module.HEAPU8.set(byteArray, ptr);

      // Call the C++ function with the pointer and size
      Module._on_file_loaded(ptr, byteArray.length);

      // Free memory later if you want to clean up
    };

    reader.readAsArrayBuffer(file);
  };

  input.click();
});

extern "C" void EMSCRIPTEN_KEEPALIVE launch_file_picker() {
    open_file_dialog();
}

#else
#include <tinyfiledialogs/tinyfiledialogs.h>
#include <filesystem>

void open_file_dialog(ShaderManager& manager) {
    const char *filters[] = { "*.png", "*.jpg", "*.jpeg", "*.bmp", "*.gif" };

    const char *filename = tinyfd_openFileDialog(
        "Choose an Image File",
        "",                  // default path
        5,                   // number of filters
        filters,
        "Image files",
        0                    // allow multiple: 0 = no
    );

    if (!filename) {
        return;
    }

    manager.add_shader(Shader<ShaderKind::Image>(std::filesystem::path(filename).filename().stem().string(), filename));
}

#endif
