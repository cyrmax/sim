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

class Audio {
  public:
    Audio() = default;
    ~Audio() = default;

    std::vector<DeviceInfo> getDevicesList();
    void selectDevice(size_t deviceIndex);

  private:
    ma_device_id m_lastDeviceID;
    std::vector<DeviceInfo> m_lastDevicesList;
};

#define g_Audio CSingleton<Audio>::GetInstance()
