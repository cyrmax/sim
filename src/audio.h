#pragma once

#include "singleton.h"

#include <climits>
#include <memory>
#include <miniaudio.h>
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
        if (ma_context_init(nullptr, 0, nullptr, &*context) != MA_SUCCESS) {
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
        if (ma_engine_init(&config, &*engine) != MA_SUCCESS) {
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
    void playAudioData();

  private:
    std::unique_ptr<CDevice> m_device;
    ma_device_id m_selectedDeviceID;
    ma_device_id m_currentDeviceID;
    std::vector<DeviceInfo> m_lastDevicesList;

    void updateDevice() {
        if (ma_device_id_equal(&m_currentDeviceID, &m_selectedDeviceID)) {
            return;
        }
        m_device = std::make_unique<CDevice>(&m_selectedDeviceID, &Audio::audioDataCallback);
        ma_device_start(*m_device);
        m_currentDeviceID = m_selectedDeviceID;
    }

    static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, const ma_uint32 frameCount) {
        auto engine = (ma_engine*)pDevice->pUserData;
        if (engine == nullptr) {
            return;
        }
        ma_engine_read_pcm_frames(engine, pOutput, frameCount, nullptr);
    }
};

#define g_Audio CSingleton<Audio>::GetInstance()
