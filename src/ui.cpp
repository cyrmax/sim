#include "ui.h"

#include "audio.h"
#include "historyStorage.h"
#include "loggerSetup.h"
#include "speech.h"

#include <spdlog/spdlog.h>
#include <string>
#include <wx/string.h>

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
    m_panel = new wxPanel(this, wxID_ANY);
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* selectionsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* settingsSizer = new wxBoxSizer(wxVERTICAL);

    auto* messageFieldLabel = new wxStaticText(m_panel, wxID_ANY, "Text to speak");
    m_messageField = new wxTextCtrl(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                    wxTE_DONTWRAP | wxTE_PROCESS_ENTER);

    auto* voicesListLabel = new wxStaticText(m_panel, wxID_ANY, "Voice");
    m_voicesList = new wxListBox(m_panel, wxID_ANY);

    auto* outputDevicesListLabel = new wxStaticText(m_panel, wxID_ANY, "Output device");
    m_outputDevicesList = new wxListBox(m_panel, wxID_ANY);

    auto* rateSliderLabel = new wxStaticText(m_panel, wxID_ANY, "Speech rate");
    m_rateSlider = new wxSlider(m_panel, wxID_ANY, 0, -10, 10);

    auto* volumeSliderLabel = new wxStaticText(m_panel, wxID_ANY, "Output volume");
    m_volumeSlider = new wxSlider(m_panel, wxID_ANY, 100, 0, 100);

    auto* voicesListSizer = new wxBoxSizer(wxVERTICAL);
    voicesListSizer->Add(voicesListLabel);
    voicesListSizer->Add(m_voicesList);
    selectionsSizer->Add(voicesListSizer);

    auto* outputDevicesListSizer = new wxBoxSizer(wxVERTICAL);
    outputDevicesListSizer->Add(outputDevicesListLabel);
    outputDevicesListSizer->Add(m_outputDevicesList);
    selectionsSizer->Add(outputDevicesListSizer);

    auto* rateSliderSizer = new wxBoxSizer(wxHORIZONTAL);
    rateSliderSizer->Add(rateSliderLabel);
    rateSliderSizer->Add(m_rateSlider);
    settingsSizer->Add(rateSliderSizer);

    auto* volumeSliderSizer = new wxBoxSizer(wxHORIZONTAL);
    volumeSliderSizer->Add(volumeSliderLabel);
    volumeSliderSizer->Add(m_volumeSlider);
    settingsSizer->Add(volumeSliderSizer);

    auto* messageFieldSizer = new wxBoxSizer(wxHORIZONTAL);
    messageFieldSizer->Add(messageFieldLabel);
    messageFieldSizer->Add(m_messageField);
    mainSizer->Add(messageFieldSizer);

    mainSizer->Add(selectionsSizer);
    mainSizer->Add(settingsSizer);
    m_messageField->SetFocus();
    m_panel->SetSizer(mainSizer);
    this->Bind(wxEVT_CHAR_HOOK, &MainFrame::OnCharEvent, this);
    m_rateSlider->Bind(wxEVT_SLIDER, &MainFrame::OnRateSliderChange, this);
    m_volumeSlider->Bind(wxEVT_SLIDER, &MainFrame::OnVolumeSliderChange, this);
    m_messageField->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnEnterPress, this);
    m_messageField->Bind(wxEVT_KEY_DOWN, &MainFrame::OnMessageFieldKeyDown, this);
    m_voicesList->Bind(wxEVT_LISTBOX, &MainFrame::OnVoiceChange, this);
    m_outputDevicesList->Bind(wxEVT_LISTBOX, &MainFrame::OnOutputDeviceChange, this);
    populateVoicesList();
    populateDevicesList();
}

void MainFrame::populateVoicesList() {
    m_voicesList->Clear();
    auto voices = Speech::GetInstance().getVoicesList();
    if (voices.empty()) {
        m_voicesList->AppendString("No voices available");
    }
    for (const auto& voiceName : voices) {
        spdlog::trace("Appending voice with name {}", voiceName);
        m_voicesList->AppendString(wxString::FromUTF8(voiceName));
    }
    m_voicesList->SetSelection(0);
}

void MainFrame::populateDevicesList() {
    m_outputDevicesList->Clear();
    auto devices = g_Audio.getDevicesList();
    if (devices.empty()) {
        m_outputDevicesList->AppendString("No devices");
        return;
    }
    for (const auto& device : devices) {
        auto isDefaultStr = device.isDefault ? "[default]" : "";
        m_outputDevicesList->AppendString(wxString::FromUTF8(std::format("{} {}", isDefaultStr, device.name)));
    }
    m_outputDevicesList->SetSelection(0);
}

void MainFrame::OnRateSliderChange(wxCommandEvent& event) {
    Speech::GetInstance().setRate(m_rateSlider->GetValue());
}

void MainFrame::OnVolumeSliderChange(wxCommandEvent& event) {
    g_Audio.setVolume(m_volumeSlider->GetValue() / 100.0f);
}

void MainFrame::OnEnterPress(wxCommandEvent& event) {
    if (m_messageField->IsEmpty()) {
        return;
    }
    wxString messageText = m_messageField->GetValue();
    std::string text = std::string(messageText.utf8_str());
    if (!Speech::GetInstance().speak(text.c_str())) {
        wxMessageBox("This voice either does not work with the program or crashes it. Please select another voice.",
                     "Error! The selected SAPI voice is not supported.", 5L, m_panel);
    }
    g_HistoryStorage.push(text);
    m_messageField->Clear();
}

void MainFrame::OnMessageFieldKeyDown(wxKeyEvent& event) {
    std::string text = std::string(m_messageField->GetValue().utf8_str());
    switch (event.GetKeyCode()) {
        case WXK_UP:
            m_messageField->SetValue(wxString::FromUTF8(g_HistoryStorage.getPreviousByText(text)));
            break;
        case WXK_DOWN:
            m_messageField->SetValue(wxString::FromUTF8(g_HistoryStorage.getNextByText(text)));
            break;
    }
    event.Skip();
}

void MainFrame::OnVoiceChange(wxCommandEvent& event) {
    int value = m_voicesList->GetSelection();
    Speech::GetInstance().setVoice(value);
}

void MainFrame::OnOutputDeviceChange(wxCommandEvent& event) {
    int value = m_outputDevicesList->GetSelection();
    g_Audio.selectDevice(value);
}

void MainFrame::OnCharEvent(wxKeyEvent& event) {
    if (event.GetKeyCode() == WXK_ESCAPE) {
        Close();
    } else {
        event.Skip();
    }
}

bool MyApp::OnInit() {
    InitializeLogging(MyApp::argc, MyApp::argv);
    if (!wxApp::OnInit()) {
        spdlog::critical("Failed to initialize WX");
        return false;
    }
    MainFrame* frame = new MainFrame("SIM test");
    frame->Show(true);
    spdlog::debug("Main window shown");
    return true;
}
