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
    if (buffer == nullptr) {
        spdlog::error("Speech buffer was nullptr");
        return false;
    }

    freeSounds();
    updateDevice();
    updateResampler(determineFormat(bitsPerSample), channels, sampleRate, AUDIO_DEFAULT_SAMPLE_RATE);
    const ma_uint64 frameCountIn = (bufferSize * 8) / (channels * bitsPerSample);
    ma_uint64 frameCountOut = 0;
    ma_result result =
        ma_resampler_get_expected_output_frame_count(&*m_resampler->resampler, frameCountIn, &frameCountOut);
    if (result != MA_SUCCESS) {
        spdlog::error("Failed to get expected frame count for resampling: {}", ma_result_description(result));
        free((void*)buffer);
        return false;
    }

    auto* pPayload = new SoundPayload();
    pPayload->pcmData.resize(frameCountOut * channels * (bitsPerSample / 8));

    if (sampleRate != AUDIO_DEFAULT_SAMPLE_RATE) {
        result = m_resampler->processAudioData(buffer, frameCountIn, pPayload->pcmData.data(), frameCountOut);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to resample audio: {}", ma_result_description(result));
            free((void*)buffer);
            delete pPayload;
            return false;
        }
        pPayload->pcmData.resize(frameCountOut * channels * (bitsPerSample / 8));
    } else {
        frameCountOut = frameCountIn;
        pPayload->pcmData.assign((uint8_t*)buffer, (uint8_t*)buffer + bufferSize);
    }

    free((void*)buffer);

    if (frameCountOut == 0) {
        delete pPayload;
        return true;
    }

    ma_audio_buffer_config config = ma_audio_buffer_config_init(determineFormat(bitsPerSample), channels, frameCountOut,
                                                                pPayload->pcmData.data(), nullptr);
    config.sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;

    pPayload->audioBuffer = std::make_unique<ma_audio_buffer>();
    result = ma_audio_buffer_init(&config, &*pPayload->audioBuffer);
    if (result != MA_SUCCESS) {
        spdlog::critical("Failed to initialize audio buffer: {}", ma_result_description(result));
        pPayload->audioBuffer.reset();
        delete pPayload;
        return false;
    }

    pPayload->sound = std::make_unique<ma_sound>();
    result = ma_sound_init_from_data_source(
        g_AudioEngine, &*pPayload->audioBuffer,
        MA_SOUND_FLAG_NO_PITCH | MA_SOUND_FLAG_NO_SPATIALIZATION | MA_SOUND_FLAG_ASYNC, nullptr, &*pPayload->sound);

    if (result != MA_SUCCESS) {
        spdlog::critical("Failed to initialize sound instance: {}", ma_result_description(result));
        ma_audio_buffer_uninit(&*pPayload->audioBuffer);
        pPayload->audioBuffer.reset();
        pPayload->sound.reset();
        delete pPayload;
        return false;
    }

    sounds.push_back(pPayload);
    ma_sound_start(&*pPayload->sound);
    return true;
}

float Audio::getVolume() {
    return ma_engine_get_volume(g_AudioEngine);
}

void Audio::setVolume(const float volume) {
    ma_engine_set_volume(g_AudioEngine, volume);
}
