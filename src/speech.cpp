#include "speech.h"

#include <spdlog/spdlog.h>

Speech::Speech() {
    spdlog::debug("SRAL instance initializing");
    if (!SRAL_IsInitialized()) {
        SRAL_Initialize(SRAL_Engines::ENGINE_NONE);
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
