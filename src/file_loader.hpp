#pragma once

#include <type_traits>
#ifndef __EMSCRIPTEN__
#include <portable-file-dialogs.h>
#endif
#include <stb/stb_image.h>

#include <optional>
#include <string>
#include <vector>

#include "context/resource.hpp"


struct FileLoader {

#ifdef __EMSCRIPTEN__
    using Callback = std::function<void(const char*, uint8_t*, size_t)>;
#else
    using Callback = std::function<void(const std::string&)>;
#endif

    template <ResourceKind K, typename CB> requires std::is_convertible_v<CB, Callback>
    bool open_dialog(CB callback);
    bool check();

  // private:
#ifndef __EMSCRIPTEN__
    std::optional<pfd::open_file> handle = std::nullopt;
#endif
    std::optional<Callback> handle_callback;
};


#ifdef __EMSCRIPTEN__
extern "C" {
    void open_file_dialog (const char* accept, void* file_loader_ptr);
}

template <ResourceKind K, typename CB> requires std::is_convertible_v<CB, FileLoader::Callback>
bool FileLoader::open_dialog(CB callback) {
    handle_callback = std::forward<CB>(callback);
    if constexpr (K == ResourceKind::Image) {
        open_file_dialog(".png,.jpg,.jpeg,.bmp", this);
    }
    return true;
}
#else
template <ResourceKind K, typename CB> requires std::is_convertible_v<CB, FileLoader::Callback>
bool FileLoader::open_dialog(CB callback) {
    if (handle.has_value()) {
        Log::warn("A dialog is already opened, file opening aborted.");
        return false;
    }

    assert(!handle.has_value());
    handle_callback = std::forward<CB>(callback);

    if constexpr (K == ResourceKind::Image) {
        handle =
            pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::multiselect);
    // } else if constexpr (K == ResourceKind::Video) {
    //     static_assert(false, "Not implemented yet.");
    }

    return true;
}
#endif
