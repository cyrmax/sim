#pragma once

#include <SRAL.h>
#include <string>
#include <vector>

class Speech {
  public:
    static Speech& GetInstance();
    // Prevent copying and moving
    Speech(const Speech&) = delete;
    Speech& operator=(const Speech&) = delete;
    Speech(Speech&&) = delete;
    Speech& operator=(Speech&&) = delete;

    std::vector<std::string> getVoicesList();
    bool speak(const char* text);
    bool setRate(uint64_t rate);
    bool setVolume(uint64_t volume);
    bool setVoice(uint64_t idx);

  private:
    Speech();
    ~Speech();
};
