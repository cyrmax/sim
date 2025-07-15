#pragma once

#include "singleton.h"

#include <climits>
#include <memory>
#include <miniaudio.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

struct DeviceInfo {
    ma_device_id id;
    std::string_view name;
    bool isDefault;
};

// Thanks to @m1maker for this idea of wrapping miniaudio in C++ way
class CAudioContext {
  public:
    CAudioContext() : context(nullptr) {
        context = std::make_unique<ma_context>();
        ma_result result = ma_context_init(nullptr, 0, nullptr, &*context);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to initialize miniaudio context: {}", ma_result_description(result));
            throw std::exception("Failed to initialize miniaudio context");
        }
    }

    ~CAudioContext() {
        ma_context_uninit(&*context);
        context.reset();
    }

    operator ma_context*() { return &*context; }

  private:
    std::unique_ptr<ma_context> context;
};

#define g_AudioContext CSingleton<CAudioContext>::GetInstance()

class CAudioEngine {
  public:
    CAudioEngine() : engine(nullptr) {
        engine = std::make_unique<ma_engine>();
        ma_engine_config config = ma_engine_config_init();
        config.noDevice = MA_TRUE;
        config.channels = 2;
        config.sampleRate = 48000;
        config.pContext = g_AudioContext;
        ma_result result = ma_engine_init(&config, &*engine);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to initialize miniaudio engine: {}", ma_result_description(result));
            throw std::exception("Failed to initialize miniaudio audio engine");
        }
    }

    ~CAudioEngine() {
        ma_engine_uninit(&*engine);
        engine.reset();
    }

    operator ma_engine*() { return &*engine; }

  private:
    std::unique_ptr<ma_engine> engine;
};

#define g_AudioEngine CSingleton<CAudioEngine>::GetInstance()

class CDevice {
  public:
    CDevice(ma_device_id* deviceID, ma_device_data_proc dataCallback) : device(nullptr) {
        device = std::make_unique<ma_device>();
        ma_device_config config = ma_device_config_init(ma_device_type_playback);
        config.playback.pDeviceID = deviceID;
        config.dataCallback = dataCallback;
        config.pUserData = g_AudioEngine;
        ma_result result = ma_device_init(g_AudioContext, &config, &*device);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to initialize audio device: {}", ma_result_description(result));
            throw std::exception();
        }
    }

    ~CDevice() {
        ma_device_uninit(&*device);
        device.reset();
    }

    operator ma_device*() { return &*device; }

  private:
    std::unique_ptr<ma_device> device;
};

class Audio {
  public:
    Audio() : m_device(nullptr) { m_selectedDeviceID = getDevicesList()[0].id; }
    ~Audio() = default;

    std::vector<DeviceInfo> getDevicesList();
    void selectDevice(size_t deviceIndex);
    bool playAudioData(const int channels, const int sampleRate, const int bitsPerSample, const uint64_t bufferSize,
                       const void* buffer);

  private:
    std::unique_ptr<CDevice> m_device;
    ma_device_id m_selectedDeviceID;
    ma_device_id m_currentDeviceID;
    std::vector<DeviceInfo> m_lastDevicesList;

    void updateDevice() {
        if (ma_device_id_equal(&m_currentDeviceID, &m_selectedDeviceID)) {
            return;
        }
        spdlog::trace("Initializing new audio device");
        m_device = std::make_unique<CDevice>(&m_selectedDeviceID, &Audio::audioDataCallback);
        ma_device_start(*m_device);
        m_currentDeviceID = m_selectedDeviceID;
    }

    static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, const ma_uint32 frameCount) {
        spdlog::trace("Audio data callback called");
        auto engine = (ma_engine*)pDevice->pUserData;
        if (engine == nullptr) {
            return;
        }
        ma_engine_read_pcm_frames(engine, pOutput, frameCount, nullptr);
    }

    struct SoundPayload {
        ma_sound sound;
        ma_audio_buffer audioBuffer;
    };

    static void soundEndCallback(void* pUserData, ma_sound* pSound) {
        spdlog::trace("Sound end callback called");
        auto payload = (SoundPayload*)pUserData;
        ma_sound_uninit(pSound);
        ma_audio_buffer_uninit(&payload->audioBuffer);
        delete payload;
    }
};

#define g_Audio CSingleton<Audio>::GetInstance()

inline ma_format determineFormat(int bitsPerSample) {
    switch (bitsPerSample) {
        case 8:
            return ma_format_u8;
        case 16:
            return ma_format_s16; // Most common for speech
        case 24:
            return ma_format_s24;
        case 32:
            return ma_format_s32;
        default:
            return ma_format_unknown;
    }
}
