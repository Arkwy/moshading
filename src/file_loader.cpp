#include "file_loader.hpp"

#include <optional>
#include <vector>

bool FileLoader::check() {
    if (handle.has_value() && handle.value().ready()) {
        assert(handle_callback.has_value());
        handle_callback.value()(*this);
        handle = std::nullopt;
        handle_callback = std::nullopt;
        return true;
    }
    return !handle.has_value();
}




// #include "file_loader.hpp"
// #ifdef __EMSCRIPTEN__

// #include <emscripten.h>
// #include <emscripten/bind.h>

// #include "log.hpp"

// extern "C" EMSCRIPTEN_KEEPALIVE void on_files_selected(const char* file, void* file_loader_ptr) {
//     FileLoader::on_files_selected(file, file_loader_ptr);
// }

// void FileLoader::on_files_selected(const char* file_, void* file_loader_ptr) {
//     std::vector<std::string> results;

//     results.push_back(file_);

//     FileLoader* file_loader = reinterpret_cast<FileLoader*>(file_loader_ptr);
//     file_loader->selected_files = std::move(results);
//     file_loader->ready = true;
// }

// // JavaScript: opens file dialog and reports result back via on_files_selected
// EM_JS(void, open_file_dialog, (const char* accept, void* file_loader_ptr), {
//     const input = document.createElement('input');
//     input.type = 'file';
//     input.multiple = false;
//     input.accept = UTF8ToString(accept);
//     input.onchange = function(e) {
//         console.log("files:", files);
//         ccall('on_files_selected', 'void', ['string', 'number'], [files, file_loader_ptr]);
//     };
//     input.click();
// });

// template <>
// bool FileLoader::open_dialog<OpenType::Image>() {
//     if (ready) return false;
//     open_file_dialog(".png,.jpg,.jpeg,.bmp", this);
//     return true;
// }

// template <>
// bool FileLoader::open_dialog<OpenType::Video>() {
//     // static_assert(false, "Video loading not yet implemented for Web.");
//     return false;
// }

// bool FileLoader::check() {
//     return ready;
// }

// std::optional<std::vector<std::string>> FileLoader::get_result() {
//     if (!ready) return std::nullopt;
//     ready = false;
//     return selected_files;
// }

// #else
// #include <optional>
// #include <vector>

// bool FileLoader::check() {
//     if (handle.has_value()) {
//         return handle.value().ready();
//     }
//     return false;
// }

// std::optional<std::vector<std::string>> FileLoader::get_result() {
//     if (check()) {
//         std::vector<std::string> result = handle.value().result();
//         handle = std::nullopt;
//         return result;
//     }
//     return std::nullopt;
// }
// #endif
