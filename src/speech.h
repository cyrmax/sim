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

  private:
    Speech();
    ~Speech();
};
