#include "speech.h"

#include "audio.h"
#include "unsupportedVoicesFilter.h"

#include <climits>
#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>

static constexpr size_t SRAL_MAX_VOICE_NAME_LEN = 128;

Speech::Speech() {
    spdlog::debug("SRAL instance initializing");
    if (!SRAL_IsInitialized()) {
        SRAL_Initialize(SRAL_ENGINE_NVDA | SRAL_ENGINE_JAWS | SRAL_ENGINE_UIA);
        spdlog::debug("SRAL initialized");
    }
}

Speech::~Speech() {
    spdlog::debug("Uninitializing SRAL");
    if (SRAL_IsInitialized()) {
        SRAL_Uninitialize();
        spdlog::debug("SRAL uninitialized");
    }
}

Speech& Speech::GetInstance() {
    static Speech instance;
    return instance;
}

std::vector<std::string> Speech::getVoicesList() {
    int voiceCount = 0;
    if (!SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_COUNT, &voiceCount)) {
        spdlog::error("Failed to get voice count from SRAL.");
        return {}; // Return an empty vector on failure.
    }
    if (voiceCount <= 0) {
        return {};
    }
    std::vector<SRAL_VoiceInfo> voiceInfos(voiceCount);
    if (!SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_PROPERTIES, voiceInfos.data())) {
        spdlog::error("Failed to get voice properties from SRAL.");
        return {};
    }
    std::vector<std::string> voices;
    voices.reserve(voiceCount);
    for (size_t i = 0; i < voiceInfos.size(); ++i) {
        bool isSupported = CheckVoiceIsSupported(voiceInfos[i]);
        if (!isSupported) {
            m_unsupportedVoiceIndices.push_back(i);
        }
        voices.emplace_back(std::format("{}{}", isSupported ? "" : "!Not supported ", voiceInfos[i].name));
    }
    return voices;
}

bool Speech::speak(const char* text) {
    if (m_unsupportedVoiceIsSet) {
        spdlog::warn("Trying to speak with unsupported voice");
        return false;
    }
    uint64_t bufferSize = 0;
    int channels = 0;
    int sampleRate = 0;
    int bitsPerSample = 0;
    auto* data = SRAL_SpeakToMemoryEx(SRAL_ENGINE_SAPI, text, &bufferSize, &channels, &sampleRate, &bitsPerSample);
    if (data == nullptr) {
        spdlog::error("SRAL_SpeakToMemoryEx returned nullptr");
        return false;
    }
    if (channels <= 0 || sampleRate <= 0 || bitsPerSample <= 0) {
        spdlog::error("SRAL returned invalid audio metadata: channels={}, sampleRate={}, bitsPerSample={}", channels,
                      sampleRate, bitsPerSample);
        free(data);
        return false;
    }
    return g_Audio.playAudioData(channels, sampleRate, bitsPerSample, bufferSize, data);
}

bool Speech::setRate(uint64_t rate) {
    return SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_RATE, &rate);
}

bool Speech::setVoice(uint64_t idx) {
    m_unsupportedVoiceIsSet = std::find(m_unsupportedVoiceIndices.begin(), m_unsupportedVoiceIndices.end(), idx) !=
                              m_unsupportedVoiceIndices.end();
    if (!SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &idx)) {
        spdlog::error("Failed to set voice index to {}", idx);
        return false;
    }
    int newIdx = 0;
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &newIdx);
    return true;
}
