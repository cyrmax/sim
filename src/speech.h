#pragma once

#include <SRAL.h>

class Speech {
  public:
    static Speech& GetInstance();

  private:
    Speech();
    ~Speech();
};
