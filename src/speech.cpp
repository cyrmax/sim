#include "speech.h"

#include "audio.h"
#include "unsuportedVoicesFilter.h"

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
    spdlog::debug("Default speech rate: {}; default speech volume: {}", m_defaultRate, g_Audio.getVolume());
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
    int voiceCount = 0;
    if (!SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_COUNT, &voiceCount)) {
        spdlog::error("Failed to get voice count from SRAL.");
        return {}; // Return an empty vector on failure.
    }
    spdlog::debug("SRAL reports {} voices", voiceCount);
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
    spdlog::trace("Speaking text: {}", text);
    uint64_t bufferSize;
    int channels;
    int sampleRate;
    int bitsPerSample;
    auto* data = SRAL_SpeakToMemoryEx(SRAL_ENGINE_SAPI, text, &bufferSize, &channels, &sampleRate, &bitsPerSample);
    g_Audio.playAudioData(channels, sampleRate, bitsPerSample, bufferSize, data);
    return true;
}

bool Speech::setRate(uint64_t rate) {
    spdlog::trace("Setting rate to {}", rate);
    return SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_SPEECH_RATE, &rate);
}

bool Speech::setVoice(uint64_t idx) {
    m_unsupportedVoiceIsSet = std::find(m_unsupportedVoiceIndices.begin(), m_unsupportedVoiceIndices.end(), idx) !=
                              m_unsupportedVoiceIndices.end();
    spdlog::trace("Setting voice ID to {}", idx);
    if (!SRAL_SetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &idx)) {
        spdlog::error("Failed to set voice index to {}", idx);
        return false;
    }
    int newIdx = 0;
    SRAL_GetEngineParameter(SRAL_ENGINE_SAPI, SRAL_PARAM_VOICE_INDEX, &newIdx);
    return true;
}
