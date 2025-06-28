#include "ui.h"

#include "audio.h"
#include "loggerSetup.h"
#include "speech.h"

#include <spdlog/spdlog.h>
#include <string>

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
    m_panel = new wxPanel(this, wxID_ANY);
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* selectionsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto* settingsSizer = new wxBoxSizer(wxVERTICAL);
    m_messageField = new wxTextCtrl(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                    wxTE_DONTWRAP | wxTE_PROCESS_ENTER);
    m_voicesList = new wxListBox(m_panel, wxID_ANY);
    m_outputDevicesList = new wxListBox(m_panel, wxID_ANY);
    m_rateSlider =
        new wxSlider(m_panel, wxID_ANY, 0, -10, 10, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
    m_volumeSlider =
        new wxSlider(m_panel, wxID_ANY, 50, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS);
    selectionsSizer->Add(m_voicesList);
    selectionsSizer->Add(m_outputDevicesList);
    settingsSizer->Add(m_rateSlider);
    settingsSizer->Add(m_volumeSlider);
    mainSizer->Add(m_messageField);
    mainSizer->Add(selectionsSizer);
    mainSizer->Add(settingsSizer);
    m_messageField->SetFocus();
    m_panel->SetSizer(mainSizer);
    this->Bind(wxEVT_CHAR_HOOK, &MainFrame::OnCharEvent, this);
    m_rateSlider->Bind(wxEVT_SLIDER, &MainFrame::OnRateSliderChange, this);
    m_volumeSlider->Bind(wxEVT_SLIDER, &MainFrame::OnVolumeSliderChange, this);
    m_messageField->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnEnterPress, this);
    m_voicesList->Bind(wxEVT_LISTBOX, &MainFrame::OnVoiceChange, this);
    populateVoicesList();
    populateDevicesList();
}

void MainFrame::populateVoicesList() {
    m_voicesList->Clear();
    auto voices = Speech::GetInstance().getVoicesList();
    spdlog::trace("Got {} voices from Speech class", voices.size());
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
        m_outputDevicesList->AppendString(std::format("{} {}", isDefaultStr, device.name));
    }
    m_outputDevicesList->SetSelection(0);
}

void MainFrame::OnRateSliderChange(wxCommandEvent& event) {
    Speech::GetInstance().setRate(m_rateSlider->GetValue());
}

void MainFrame::OnVolumeSliderChange(wxCommandEvent& event) {
    Speech::GetInstance().setVolume(m_volumeSlider->GetValue());
}

void MainFrame::OnEnterPress(wxCommandEvent& event) {
    Speech::GetInstance().speak(m_messageField->GetValue().ToUTF8().data());
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
