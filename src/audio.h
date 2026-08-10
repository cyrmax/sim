#pragma once

#include "singleton.h"

#include <climits>
#include <cstring>
#include <memory>
#include <miniaudio.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <vector>

inline constexpr ma_uint32 AUDIO_DEFAULT_SAMPLE_RATE = 48000;

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

/*
This declaration is made to free resources in the right order.
We deinitialize SoundPayloads first, then AudioEngine.
*/
static void freeAllSounds();

class CAudioEngine {
  public:
    CAudioEngine() : engine(nullptr) {
        engine = std::make_unique<ma_engine>();
        ma_engine_config config = ma_engine_config_init();
        config.noDevice = MA_TRUE;
        config.channels = 2;
        config.sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;
        config.pContext = g_AudioContext;
        ma_result result = ma_engine_init(&config, &*engine);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to initialize miniaudio engine: {}", ma_result_description(result));
            throw std::exception("Failed to initialize miniaudio audio engine");
        }
    }

    ~CAudioEngine() {
        freeAllSounds();
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
        config.sampleRate = AUDIO_DEFAULT_SAMPLE_RATE;

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

class CResampler {
  public:
    CResampler(ma_format format, ma_uint32 channels, ma_uint32 sampleRateIn, ma_uint32 sampleRateOut) {
        resampler = std::make_unique<ma_resampler>();
        ma_resampler_config config =
            ma_resampler_config_init(format, channels, sampleRateIn, sampleRateOut, ma_resample_algorithm_linear);

        ma_result result = ma_resampler_init(&config, nullptr, &*resampler);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to initialize resampler: {}", ma_result_description(result));
            throw std::exception();
        }
    }

    ~CResampler() {
        ma_resampler_uninit(&*resampler, nullptr);
        resampler.reset();
    }

    operator ma_resampler*() { return &*resampler; }

    ma_result setRate(ma_uint32 sampleRateIn, ma_uint32 sampleRateOut) {
        return resampler != nullptr ? ma_resampler_set_rate(&*resampler, sampleRateIn, sampleRateOut) : MA_ERROR;
    }

    ma_result processAudioData(const void* pFramesIn, ma_uint64 frameCountIn, void* pFramesOut,
                               ma_uint64& frameCountOut) {
        if (pFramesIn == nullptr || pFramesOut == nullptr) {
            return MA_INVALID_ARGS;
        }

        if (resampler == nullptr) {
            return MA_ERROR;
        }

        ma_uint64 inFrames = frameCountIn;
        return ma_resampler_process_pcm_frames(&*resampler, pFramesIn, &inFrames, pFramesOut, &frameCountOut);
    }

  private:
    std::unique_ptr<ma_resampler> resampler;
    friend class Audio;
};

class Audio {
  public:
    Audio() : m_device(nullptr), m_hasCurrentDevice(false) {
        auto devices = getDevicesList();
        if (devices.empty()) {
            spdlog::warn("No audio devices found during Audio initialization");
            std::memset(&m_selectedDeviceID, 0, sizeof(m_selectedDeviceID));
            std::memset(&m_currentDeviceID, 0, sizeof(m_currentDeviceID));
            return;
        }
        m_selectedDeviceID = devices[0].id;
        std::memset(&m_currentDeviceID, 0, sizeof(m_currentDeviceID));
    }
    ~Audio() = default;

    std::vector<DeviceInfo> getDevicesList();
    void selectDevice(size_t deviceIndex);
    bool playAudioData(const int channels, const int sampleRate, const int bitsPerSample, const uint64_t bufferSize,
                       const void* buffer);
    float getVolume();
    void setVolume(const float volume);

  private:
    std::unique_ptr<CDevice> m_device;
    std::unique_ptr<CResampler> m_resampler;
    ma_device_id m_selectedDeviceID;
    ma_device_id m_currentDeviceID;
    bool m_hasCurrentDevice;
    std::vector<DeviceInfo> m_lastDevicesList;

    void updateDevice() {
        if (m_hasCurrentDevice && ma_device_id_equal(&m_currentDeviceID, &m_selectedDeviceID)) {
            return;
        }
        spdlog::debug("Initializing new audio device");
        m_device = std::make_unique<CDevice>(&m_selectedDeviceID, &Audio::audioDataCallback);
        ma_device_start(*m_device);
        m_currentDeviceID = m_selectedDeviceID;
        m_hasCurrentDevice = true;
    }

    void updateResampler(ma_format format, ma_uint32 channels, ma_uint32 sampleRateIn, ma_uint32 sampleRateOut) {
        if (m_resampler == nullptr || m_resampler->resampler->format != format ||
            m_resampler->resampler->channels != channels) {
            m_resampler = std::make_unique<CResampler>(format, channels, sampleRateIn, sampleRateOut);
            return;
        }

        if (sampleRateIn == sampleRateOut)
            return;

        ma_result result = m_resampler->setRate(sampleRateIn, sampleRateOut);
        if (result != MA_SUCCESS) {
            spdlog::error("Failed to set sample rate: {}", ma_result_description(result));
            throw std::exception();
        }
    }

    static void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, const ma_uint32 frameCount) {
        auto engine = (ma_engine*)pDevice->pUserData;
        if (engine == nullptr) {
            return;
        }
        ma_engine_read_pcm_frames(engine, pOutput, frameCount, nullptr);
    }

    struct SoundPayload {
        std::unique_ptr<ma_sound> sound;
        std::unique_ptr<ma_audio_buffer> audioBuffer;
        std::vector<ma_uint8> pcmData;

        ~SoundPayload() {
            if (sound != nullptr) {
                ma_sound_uninit(&*sound);
                sound.reset();
            }

            if (audioBuffer != nullptr) {
                ma_audio_buffer_uninit(&*audioBuffer);
                audioBuffer.reset();
            }
        }
    };

    std::vector<SoundPayload*> sounds;

  public:
    void freeSounds(bool onlyUnused = true) {
        int counter = 0;
        auto it = std::remove_if(sounds.begin(), sounds.end(), [&](SoundPayload* sound) {
            if (sound != nullptr && (!onlyUnused || (sound->sound != nullptr && ma_sound_at_end(&*sound->sound)))) {
                delete sound;
                counter++;
                return true;
            }
            return false;
        });

        sounds.erase(it, sounds.end()); // Erase the removed elements from the vector

        if (counter > 0) {
            spdlog::debug("Sounds freed: {}", counter);
        }
    }
};

#define g_Audio CSingleton<Audio>::GetInstance()

static void freeAllSounds() {
    g_Audio.freeSounds(false);
}

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
