#include "historyStorage.h"

#include <algorithm>

void HistoryStorage::push(const std::string& text) {
    auto iter = std::find(m_messages.begin(), m_messages.end(), text);
    if (iter != m_messages.end()) {
        m_messages.erase(iter);
    }
    m_messages.push_back(text);
}

std::string HistoryStorage::getNextByText(const std::string& text) {
    if (m_messages.empty()) {
        return "";
    }
    if (text.empty()) {
        return "";
    }
    if (m_messages.size() == 1) {
        return "";
    }
    auto iter = std::find(m_messages.begin(), m_messages.end(), text);
    if (iter == m_messages.end()) {
        return "";
    }
    auto idx = std::distance(m_messages.begin(), iter);
    if (idx >= (m_messages.size() - 1)) {
        return "";
    }
    return m_messages[idx + 1];
}

std::string HistoryStorage::getPreviousByText(const std::string& text) {
    if (m_messages.empty()) {
        return "";
    }
    if (text.empty()) {
        return m_messages[m_messages.size() - 1];
    }
    auto iter = std::find(m_messages.begin(), m_messages.end(), text);
    if (iter == m_messages.end()) {
        return "";
    }
    auto idx = std::distance(m_messages.begin(), iter);
    if (idx <= 0) {
        return m_messages[0];
    }
    return m_messages[idx - 1];
}
