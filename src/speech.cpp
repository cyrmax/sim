#include "speech.h"

#include <climits>
#include <spdlog/spdlog.h>

Speech::Speech() {
    spdlog::debug("SRAL instance initializing");
    if (!SRAL_IsInitialized()) {
        SRAL_Initialize(SRAL_Engines::ENGINE_NVDA | SRAL_Engines::ENGINE_JAWS | SRAL_Engines::ENGINE_UIA);
        spdlog::debug("SRAL initialized");
    }
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
    auto voiceCount = SRAL_GetVoiceCountEx(SRAL_Engines::ENGINE_SAPI);
    spdlog::trace("SRAL reports {} voices", voiceCount);
    std::vector<std::string> voices;
    voices.reserve(voiceCount);
    for (size_t i = 0; i < voiceCount; ++i) {
        voices.push_back(SRAL_GetVoiceNameEx(SRAL_Engines::ENGINE_SAPI, i));
        spdlog::trace("Got voice number {}: name {}", i, voices[i]);
    }
    return voices;
}

bool Speech::speak(const char* text) {
    spdlog::trace("Speaking text: {}", text);
    return SRAL_SpeakEx(SRAL_Engines::ENGINE_SAPI, text, true);
}

bool Speech::setRate(uint64_t rate) {
    spdlog::trace("Setting rate to {}", rate);
    return SRAL_SetRateEx(ENGINE_SAPI, rate);
}

bool Speech::setVolume(uint64_t volume) {
    spdlog::trace("Setting volume to {}", volume);
    return SRAL_SetVolumeEx(ENGINE_SAPI, volume);
}

bool Speech::setVoice(uint64_t idx) {
    spdlog::trace("Setting voice ID to {}", idx);
    return SRAL_SetVoiceEx(SRAL_Engines::ENGINE_SAPI, idx);
}
