#include "file_loader.hpp"

#include <cstring>
#include <vector>

#include "src/context/resource.hpp"

#ifdef __EMSCRIPTEN__

    #include <emscripten.h>
    #include <emscripten/bind.h>

extern "C" EMSCRIPTEN_KEEPALIVE void on_files_selected(
    const char* name, uint8_t* file, size_t len, void* file_loader_ptr
) {
    FileLoader fl = *reinterpret_cast<FileLoader*>(file_loader_ptr);
    assert(fl.handle_callback.has_value());
    fl.handle_callback.value()(name, file, len);
}


EM_JS(void, open_file_dialog, (const char* accept, void* file_loader_ptr), {
    const input = document.createElement('input');
    input.type = 'file';
    input.accept = UTF8ToString(accept);
    input.multiple = true;
    input.onchange = async() => {
        const files = Array.from(input.files);
        for (const file of files) {
            const buffer = await file.arrayBuffer();
            const byteArray = new Uint8Array(buffer);
            const ptr = _malloc(byteArray.length);
            const stem = file.name.split('.').slice(0, -1).join('.');
            HEAPU8.set(byteArray, ptr);
            ccall(
                'on_files_selected',
                'void',
                [ 'string', 'number', 'number', 'number' ],
                [ stem, ptr, byteArray.length, file_loader_ptr ]
            );
        }
    };

    input.click();
});

bool FileLoader::check() {
    return true;  // page should be frozen when dialog is open anyway
}

#else

    #include <optional>

bool FileLoader::check() {
    if (handle.has_value() && handle.value().ready()) {
        assert(handle_callback.has_value());
        std::vector<std::string> files = handle.value().result();
        for (auto& file : files) {
            handle_callback.value()(file);
        }
        handle = std::nullopt;
        handle_callback = std::nullopt;
        return true;
    }
    return !handle.has_value();
}

#endif
