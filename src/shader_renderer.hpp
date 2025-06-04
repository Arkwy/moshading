#pragma once
#include "webgpu/webgpu-raii.hpp"

struct ShaderRenderer {

  private:
    bool initialized = false;

    void initialize();
};
