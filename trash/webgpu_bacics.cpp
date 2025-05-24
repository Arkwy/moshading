#include <iostream>
#include <webgpu/webgpu-raii.hpp>

inline std::ostream& operator<<(std::ostream& os, const wgpu::StringView& wsv) {
    os.write(wsv.data, wsv.length);
    return os;
}


void inspect_adapter(const wgpu::Adapter& adapter) {
    wgpu::Limits limits;
    if (adapter.getLimits(&limits) == wgpu::Status::Success) {
        std::cout << "Adapter limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
    }
    wgpu::SupportedFeatures supported_features;
    adapter.getFeatures(&supported_features);

    std::cout << "Adapter features:" << std::endl;
    std::cout << std::hex;
    for (size_t i = 0; i < supported_features.featureCount; i++) {
        std::cout << "- 0x" << supported_features.features[i] << std::endl;
    }
    std::cout << std::dec;

    wgpu::AdapterInfo adapter_info;
    adapter.getInfo(&adapter_info);

    std::cout << "Adapter properties:" << std::endl;
    std::cout << " - vendorID: " << adapter_info.vendorID << std::endl;
    if (adapter_info.vendor.length) {
        std::cout << " - vendorName: " << adapter_info.vendor << std::endl;
    }
    if (adapter_info.architecture.length) {
        std::cout << " - architecture: " << adapter_info.architecture << std::endl;
    }
    std::cout << " - deviceID: " << adapter_info.deviceID << std::endl;
    if (adapter_info.device.length) {
        std::cout << " - device: " << adapter_info.device << std::endl;
    }
    if (adapter_info.description.length) {
        std::cout << " - driverDescription: " << adapter_info.description << std::endl;
    }
    std::cout << std::hex;
    std::cout << " - adapterType: 0x" << adapter_info.adapterType << std::endl;
    std::cout << " - backendType: 0x" << adapter_info.backendType << std::endl;
    std::cout << std::dec;
}

void inspect_device(wgpu::Device device) {
    wgpu::Limits limits;
    if (device.getLimits(&limits) == wgpu::Status::Success) {
        std::cout << "Device limits:" << std::endl;
        std::cout << " - maxTextureDimension1D: " << limits.maxTextureDimension1D << std::endl;
        std::cout << " - maxTextureDimension2D: " << limits.maxTextureDimension2D << std::endl;
        std::cout << " - maxTextureDimension3D: " << limits.maxTextureDimension3D << std::endl;
        std::cout << " - maxTextureArrayLayers: " << limits.maxTextureArrayLayers << std::endl;
        {
            {
            }
        }
    }

    wgpu::SupportedFeatures supported_features;
    device.getFeatures(&supported_features);

    std::cout << "Device features:" << std::endl;
    std::cout << std::hex;
    for (size_t i = 0; i < supported_features.featureCount; i++) {
        std::cout << " - 0x" << supported_features.features[i] << std::endl;
    }
    std::cout << std::dec;
}


void webgpu_basics() {

}
