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
