#pragma once

#include "singleton.h"

#include <string>
#include <vector>

class HistoryStorage {
  public:
    void push(const std::string& text);
    std::string getNextByText(const std::string& text);
    std::string getPreviousByText(const std::string& text);

  private:
    std::vector<std::string> m_messages;
};

#define g_HistoryStorage CSingleton<HistoryStorage>::GetInstance()
