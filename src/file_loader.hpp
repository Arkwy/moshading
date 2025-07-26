// #pragma once

// #ifndef __EMSCRIPTEN__
//     #include <portable-file-dialogs.h>
// #endif
// #include <stb/stb_image.h>

// #include <optional>
// #include <string>
// #include <vector>

// #include "context/resource.hpp"


// template <ResourceKind T>
// struct FileLoader {
//     std::vector<Resource<T>> ressources;


//     bool check();
//     std::optional<std::vector<std::string>> get_result();

//     template <ResourceKind OT>
//     bool open_dialog();

// #ifdef __EMSCRIPTEN__
//     static void on_files_selected(const char* file_, void* file_loader_ptr);

//   private:
//     bool ready = false;
//     std::vector<std::string> selected_files;
// #else
//     std::optional<pfd::open_file> handle = std::nullopt;
// #endif
// };





////////////// OLD ///////////////
// #ifndef __EMSCRIPTEN__
// template <OpenType OT>
// bool FileLoader::open_dialog() {
//     if (handle.has_value()) {
//         Log::warn("A dialog is already opened, file opening aborted.2");
//         return false;
//     }

//     if constexpr (OT == OpenType::Image) {
//         Log::warn("should dialog");
//         handle =
//             pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
//     } else if constexpr (OT == OpenType::Video) {
//         static_assert(false, "Not implemented yet.");
//     }

//     return true;
// }
// #endif

// #pragma once

// #ifndef __EMSCRIPTEN__
//     #include <portable-file-dialogs.h>
// #endif

// #include <optional>
// #include <string>
// #include <vector>

// #include "log.hpp"

// enum class OpenType {
//     Image,
//     Video,
// };

// struct FileLoader {
//     bool check();
//     std::optional<std::vector<std::string>> get_result();

//     template <OpenType OT>
//     bool open_dialog();

// #ifdef __EMSCRIPTEN__
//     static void on_files_selected(const char* file_, void* file_loader_ptr);
//   private:
//     bool ready = false;
//     std::vector<std::string> selected_files;
// #else
//     std::optional<pfd::open_file> handle = std::nullopt;
// #endif
// };

// #ifndef __EMSCRIPTEN__
// template <OpenType OT>
// bool FileLoader::open_dialog() {
//     if (handle.has_value()) {
//         Log::warn("A dialog is already opened, file opening aborted.2");
//         return false;
//     }

//     if constexpr (OT == OpenType::Image) {
//         Log::warn("should dialog");
//         handle =
//             pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
//     } else if constexpr (OT == OpenType::Video) {
//         static_assert(false, "Not implemented yet.");
//     }

//     return true;
// }
// #endif
