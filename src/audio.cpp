#include "audio.h"

std::vector<DeviceInfo> Audio::getDevicesList() {
    std::vector<DeviceInfo> deviceInfos;
    ma_device_info* pDeviceInfos;
    ma_uint32 deviceCount;
    ma_result result = ma_context_get_devices(g_AudioContext, &pDeviceInfos, &deviceCount, nullptr, nullptr);
    if (result != MA_SUCCESS) {
        throw std::exception("Failed to get list of devices");
    }
    deviceInfos.reserve(deviceCount);
    for (ma_uint32 i = 0; i < deviceCount; ++i) {
        deviceInfos.push_back(
            DeviceInfo(pDeviceInfos[i].id, pDeviceInfos[i].name, pDeviceInfos[i].isDefault == MA_TRUE));
    }
    return deviceInfos;
}
