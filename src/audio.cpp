#include "audio.h"

#include <algorithm>

std::vector<DeviceInfo> Audio::getDevicesList() {
    std::vector<DeviceInfo> deviceInfos;
    ma_device_info* pDeviceInfos;
    ma_uint32 deviceCount;
    ma_result result = ma_context_get_devices(g_AudioContext, &pDeviceInfos, &deviceCount, nullptr, nullptr);
    if (result != MA_SUCCESS) {
        spdlog::critical("Failed to get list of devices");
        throw std::exception("Failed to get list of devices");
    }
    deviceInfos.reserve(deviceCount);
    for (ma_uint32 i = 0; i < deviceCount; ++i) {
        deviceInfos.push_back(
            DeviceInfo(pDeviceInfos[i].id, pDeviceInfos[i].name, pDeviceInfos[i].isDefault == MA_TRUE));
    }
    m_lastDevicesList = deviceInfos;
    std::sort(m_lastDevicesList.begin(), m_lastDevicesList.end(),
              [](const DeviceInfo& first, const DeviceInfo& second) { return first.isDefault > second.isDefault; });
    return m_lastDevicesList;
}

void Audio::selectDevice(size_t deviceIndex) {
    m_selectedDeviceID = m_lastDevicesList[deviceIndex].id;
}

bool Audio::playAudioData(const int channels, const int sampleRate, const int bitsPerSample, const uint64_t bufferSize,
                          const void* buffer) {
    updateDevice();
    const ma_uint64 frameCount = bufferSize / (channels * bitsPerSample / 8);
    auto* pPayload = new SoundPayload();
    auto config = ma_audio_buffer_config_init(determineFormat(bitsPerSample), channels, frameCount, buffer, nullptr);
    config.sampleRate = sampleRate;
    ma_result result = ma_audio_buffer_init(&config, &pPayload->audioBuffer);
    if (result != MA_SUCCESS) {
        spdlog::critical("Failed to initialize audio buffer");
        return false;
    }
    result = ma_sound_init_from_data_source(
        g_AudioEngine, &pPayload->audioBuffer,
        MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_ASYNC, nullptr, &pPayload->sound);
    if (result != MA_SUCCESS) {
        spdlog::critical("Failed to initialize sound instance");
        ma_audio_buffer_uninit(&pPayload->audioBuffer);
        return false;
    }
    ma_sound_start(&pPayload->sound);
    ma_sound_set_end_callback(&pPayload->sound, soundEndCallback, pPayload);
    return true;
}
