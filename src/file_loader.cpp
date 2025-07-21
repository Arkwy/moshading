#ifdef __EMSCRIPTEN__
#include "file_loader.hpp"

#include <emscripten.h>
#include <emscripten/bind.h>

// Global pointer to current FileLoader instance (basic signal bridge)
static FileLoader* current_loader = nullptr;

// Called from JS when files are selected
extern "C" void on_files_selected(const char* files_csv) {
    if (!current_loader) return;

    std::string csv(files_csv);
    std::vector<std::string> results;
    size_t start = 0;
    size_t end;

    while ((end = csv.find(';', start)) != std::string::npos) {
        results.push_back(csv.substr(start, end - start));
        start = end + 1;
    }
    if (start < csv.length()) results.push_back(csv.substr(start));

    current_loader->selected_files = std::move(results);
    current_loader->ready = true;
}

// JavaScript: opens file dialog and reports result back via on_files_selected
EM_JS(void, open_file_dialog, (const char* accept), {
    const input = document.createElement('input');
    input.type = 'file';
    input.multiple = true;
    input.accept = UTF8ToString(accept);
    input.onchange = function(e) {
        const files = Array.from(input.files).map(f => f.name).join(';');
        ccall('on_files_selected', 'void', ['string'], [files]);
    };
    input.click();
});

template <>
bool FileLoader::open_dialog<OpenType::Image>() {
    if (current_loader != nullptr || ready) return false;

    current_loader = this;
    open_file_dialog(".png,.jpg,.jpeg,.bmp");
    return true;
}

template <>
bool FileLoader::open_dialog<OpenType::Video>() {
    // static_assert(false, "Video loading not yet implemented for Web.");
    return false;
}

bool FileLoader::check() {
    return ready;
}

std::optional<std::vector<std::string>> FileLoader::get_result() {
    if (!ready) return std::nullopt;
    ready = false;
    current_loader = nullptr;
    return selected_files;
}

#else
#include <optional>
#include <vector>

#include "file_loader.hpp"

bool FileLoader::check() {
    if (handle.has_value()) {
        return handle.value().ready();
    }
    return false;
}

std::optional<std::vector<std::string>> FileLoader::get_result() {
    if (check()) {
        std::vector<std::string> result = handle.value().result();
        handle = std::nullopt;
        return result;
    }
    return std::nullopt;
}
#endif
