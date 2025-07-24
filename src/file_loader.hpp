#pragma once

#ifndef __EMSCRIPTEN__
#include <portable-file-dialogs.h>
#endif

#include <optional>
#include <string>
#include <vector>

#include "log.hpp"
enum class OpenType {
    Image,
    Video,
};

struct FileLoader {
    bool check();
    std::optional<std::vector<std::string>> get_result();

    template <OpenType OT>
    bool open_dialog();

    #ifdef __EMSCRIPTEN__
        bool ready = false;
        std::vector<std::string> selected_files;
    #else
  private:
    std::optional<pfd::open_file> handle = std::nullopt;
    #endif
};

#ifndef __EMSCRIPTEN__
template <OpenType OT>
bool FileLoader::open_dialog() {
    if (handle.has_value()) {
        Log::warn("A dialog is already opened, file opening aborted.2");
        return false;
    }

    if constexpr (OT == OpenType::Image) {
        Log::warn("should dialog");
        handle = pfd::open_file("Select an Image File", ".", {"Image Files", "*.png *.jpg *.jpeg *.bmp"}, pfd::opt::none);
    } else if constexpr (OT == OpenType::Video) {
        static_assert(false, "Not implemented yet.");
    }

    return true;
}
#endif
