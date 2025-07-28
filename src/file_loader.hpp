#pragma once

#include <portable-file-dialogs.h>
#include <stb/stb_image.h>

#include <optional>
#include <string>
#include <vector>

#include "context/resource.hpp"


struct FileLoader {

    using Callback = std::function<void(FileLoader&)>;

    template <ResourceKind K>
    bool open_dialog(Callback&& callback);
    bool check();

  // private:
    std::optional<pfd::open_file> handle = std::nullopt;
    std::optional<Callback> handle_callback;
};


template <ResourceKind K>
bool FileLoader::open_dialog(Callback&& callback) {
    if (handle.has_value()) {
        Log::warn("A dialog is already opened, file opening aborted.");
        return false;
    }

    assert(!handle.has_value());
    handle_callback = std::move(callback);

    if constexpr (K == ResourceKind::Image) {
        Log::warn("should dialog");
        handle =
            pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
    // } else if constexpr (K == ResourceKind::Video) {
    //     static_assert(false, "Not implemented yet.");
    }

    return true;
}



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
// template <ResourceKind OT>
// bool FileLoader::open_dialog() {
//     if (handle.has_value()) {
//         Log::warn("A dialog is already opened, file opening aborted.2");
//         return false;
//     }

//     if constexpr (OT == ResourceKind::Image) {
//         Log::warn("should dialog");
//         handle =
//             pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
//     } else if constexpr (OT == ResourceKind::Video) {
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

// enum class ResourceKind {
//     Image,
//     Video,
// };

// struct FileLoader {
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

// #ifndef __EMSCRIPTEN__
// template <ResourceKind OT>
// bool FileLoader::open_dialog() {
//     if (handle.has_value()) {
//         Log::warn("A dialog is already opened, file opening aborted.2");
//         return false;
//     }

//     if constexpr (OT == ResourceKind::Image) {
//         Log::warn("should dialog");
//         handle =
//             pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
//     } else if constexpr (OT == ResourceKind::Video) {
//         static_assert(false, "Not implemented yet.");
//     }

//     return true;
// }
// #endif
