#pragma once

#include <SRAL.h>
#include <map>
#include <string_view>

struct VoiceInfo {
    uint64_t id;
    std::string_view name;
};

class Speech {
  public:
    static Speech& GetInstance();

    std::map<uint64_t, std::string_view> getVoicesList();

  private:
    static Speech* sharedInstance;
    Speech();
    ~Speech();
};
