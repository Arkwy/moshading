#include <iostream>
#include <vector>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>
#include <emscripten/bind.h>

// Callback from JS with file data
extern "C" void on_file_loaded(uintptr_t dataPtr, int size) {
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
#include <fstream>
#include <tinyfiledialogs/tinyfiledialogs.h>

void open_file_dialog() {
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
        std::cout << "No file selected.\n";
        return;
    }

    std::cout << "Selected file: " << filename << std::endl;

    // Load file into memory
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Error opening file.\n";
        return;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<unsigned char> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Error reading file.\n";
        return;
    }

    std::cout << "File loaded, size: " << size << " bytes\n";
    std::cout << "First 10 bytes: ";
    for (int i = 0; i < std::min<size_t>(10, buffer.size()); ++i) {
        std::cout << std::hex << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
}

#endif
