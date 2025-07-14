// #pragma once

// #include <tuple>
// #include <unordered_map>
// #include <utility>
// #include <vector>
// #include "src/file_loader.hpp"
// #include "shaders/circle.hpp"
// #include "shaders/chromatic_aberration.hpp"
// #include "shaders/image.hpp"
// #include "shaders/noise.hpp"
// #include "shaders/dithering.hpp"
// #include "src/shader/shader.hpp"

// struct ShaderCollection {
//     std::unordered_map<ShaderKind, ShaderSource> sources; 
//     FileLoader file_loader;

//     ShaderCollection(): sources(), file_loader() {}


//     template <ShaderKind K>
//     ShaderSource get() {
//         if (sources.contains(K)) {
//             return sources[K];
//         } else {
//             sources.emplace(std::piecewise_construct, std::forward_as_tuple(K), std::forward_as_tuple());
//         }
//     }
// };
