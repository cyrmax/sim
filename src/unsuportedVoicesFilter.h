#pragma once

#include <SRAL.h>
#include <cstring>

inline bool CheckVoiceIsSupported(const SRAL_VoiceInfo& voiceInfo) {
    const char* voiceName = voiceInfo.name;
    const char* voiceVendor = (voiceInfo.vendor != nullptr) ? voiceInfo.vendor : "Totally Supported Vendor";
    // Hungarian Profivox does not support speaking into memory and crashes the application
    if (strcmp(voiceVendor, "BME-TMIT") == 0) {
        return false;
    }
    // NVDA SAPI speaks with NVDA and so cannot speak into memory
    // We match both by vendor and name in case if there is another NVDA SAPI implementation from another developer
    if (strcmp(voiceName, "Nvda Sapi") == 0 && strcmp(voiceVendor, "Kowtom") == 0) {
        return false;
    }
    return true;
}
