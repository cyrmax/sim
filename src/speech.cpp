#include "speech.h"

#include <climits>
#include <spdlog/spdlog.h>
#include <vector>

Speech* Speech::sharedInstance = nullptr;

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
    if (!sharedInstance) {
        sharedInstance = new Speech();
    }
    return *sharedInstance;
}

std::map<uint64_t, std::string_view> Speech::getVoicesList() {
    auto voiceCount = SRAL_GetVoiceCountEx(SRAL_Engines::ENGINE_SAPI);
    std::map<uint64_t, std::string_view> voices;
    for (size_t i = 0; i < voiceCount; ++i) {
        voices[i] = SRAL_GetVoiceNameEx(SRAL_Engines::ENGINE_SAPI, i);
    }
    return voices;
}
