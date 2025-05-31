#include "speech.h"

#include <climits>
#include <memory>
#include <spdlog/spdlog.h>

static constexpr size_t SRAL_MAX_VOICE_NAME_LEN = 128;

Speech::Speech() {
    spdlog::debug("SRAL instance initializing");
    if (!SRAL_IsInitialized()) {
        SRAL_Initialize(SRAL_ENGINE_NVDA | SRAL_ENGINE_JAWS | SRAL_ENGINE_UIA);
        spdlog::debug("SRAL initialized");
    }
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_RATE, &m_defaultRate);
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_VOLUME, &m_defaultVolume);
    spdlog::debug("Default speech rate: {}; default speech volume: {}", m_defaultRate, m_defaultVolume);
}

Speech::~Speech() {
    spdlog::debug("SRAL destructor called");
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
    spdlog::trace("Getting SAPI voice list");
    int voiceCount = 0;
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_COUNT, &voiceCount);
    spdlog::trace("SRAL reports {} voices", voiceCount);
    std::vector<std::string> voices;
    std::vector<char*> c_voice_names_ptrs(voiceCount);
    std::vector<std::unique_ptr<char[]>> voiceNameBuffers;
    voiceNameBuffers.reserve(voiceCount);
    for (size_t i = 0; i < voiceCount; ++i) {
        auto buffer = std::make_unique<char[]>(SRAL_MAX_VOICE_NAME_LEN);
        buffer[0] = '\0';
        c_voice_names_ptrs[i] = buffer.get();
        voiceNameBuffers.push_back(std::move(buffer));
    }
    if (!SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_LIST, c_voice_names_ptrs.data())) {
        return voices;
    }
    voices.reserve(voiceCount);
    for (size_t i = 0; i < voiceCount; ++i) {
        voices.emplace_back(c_voice_names_ptrs[i]);
    }
    return voices;
}

bool Speech::speak(const char* text) {
    spdlog::trace("Speaking text: {}", text);
    if (!SRAL_SpeakEx(SRAL_ENGINE_SAPI, text, true)) {
        spdlog::error("Error speaking");
        return false;
    }
    return true;
}

bool Speech::setRate(uint64_t rate) {
    spdlog::trace("Setting rate to {}", rate);
    return SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_RATE, &rate);
}

bool Speech::setVolume(uint64_t volume) {
    spdlog::trace("Setting volume to {}", volume);
    return SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_VOLUME, &volume);
}

bool Speech::setVoice(uint64_t idx) {
    spdlog::trace("Setting voice ID to {}", idx);
    if (!SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &idx)) {
        spdlog::error("Failed to set voice index to {}", idx);
        return false;
    }
    int newIdx = 0;
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &newIdx);
    spdlog::trace("After set the new voice number is {}", newIdx);
    return true;
}
