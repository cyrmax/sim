#pragma once

#include <SRAL.h>
#include <string>
#include <vector>

class Speech {
  public:
    static Speech& GetInstance();

    std::vector<std::string> getVoicesList();

  private:
    static Speech* sharedInstance;
    Speech();
    ~Speech();
};
