#include "ui.h"

#include "audio.h"
#include "historyStorage.h"
#include "loggerSetup.h"
#include "speech.h"

#include <CLI/CLI.hpp>
#include <cstring>
#include <spdlog/spdlog.h>
#include <string>
#include <wx/clipbrd.h>
#include <wx/string.h>

static std::string PROGRAM_TITLE = std::format("SIM {}", SIM_FULL_VERSION);

MainFrame::MainFrame(const wxString& title, int cliVoiceIndex, std::string cliVoiceName, int cliOutputDeviceIndex,
                     std::string helpText)
    : wxFrame(nullptr, wxID_ANY, title) {
    m_cliVoiceIndex = cliVoiceIndex;
    m_cliVoiceName = cliVoiceName;
    m_cliOutputDeviceIndex = cliOutputDeviceIndex;
    m_helpText = helpText;

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

    m_helpButton = new wxButton(m_panel, wxID_ANY, "Command Line Help");

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
    mainSizer->Add(m_helpButton);

    m_messageField->SetFocus();
    m_panel->SetSizer(mainSizer);
    this->Bind(wxEVT_CHAR_HOOK, &MainFrame::OnCharEvent, this);
    m_rateSlider->Bind(wxEVT_SLIDER, &MainFrame::OnRateSliderChange, this);
    m_volumeSlider->Bind(wxEVT_SLIDER, &MainFrame::OnVolumeSliderChange, this);
    m_messageField->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnEnterPress, this);
    m_messageField->Bind(wxEVT_KEY_DOWN, &MainFrame::OnMessageFieldKeyDown, this);
    m_voicesList->Bind(wxEVT_LISTBOX, &MainFrame::OnVoiceChange, this);
    m_outputDevicesList->Bind(wxEVT_LISTBOX, &MainFrame::OnOutputDeviceChange, this);
    m_helpButton->Bind(wxEVT_BUTTON, &MainFrame::OnHelpButton, this);

    populateVoicesList();
    populateDevicesList();
}

void MainFrame::populateVoicesList() {
    m_voicesList->Clear();
    auto voices = Speech::GetInstance().getVoicesList();
    if (voices.empty()) {
        m_voicesList->AppendString("No voices available");
        spdlog::warn("No voices available, voice selection is disabled");
        return;
    }
    size_t voiceCounter = 0;
    bool isVoiceFoundByName = false;
    for (const auto& voiceName : voices) {
        m_voicesList->AppendString(wxString::FromUTF8(voiceName));
        if (!isVoiceFoundByName && voiceName == m_cliVoiceName) {
            m_cliVoiceIndex = voiceCounter;
            isVoiceFoundByName = true;
        }
        voiceCounter++;
    }
    if (m_cliVoiceIndex < 0 || static_cast<size_t>(m_cliVoiceIndex) >= voices.size()) {
        spdlog::warn("Voice index {} is out of range. Falling back to 0.", m_cliVoiceIndex);
        m_cliVoiceIndex = 0;
    }
    m_voicesList->SetSelection(m_cliVoiceIndex);
    Speech::GetInstance().setVoice(static_cast<uint64_t>(m_cliVoiceIndex));
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
    if (m_cliOutputDeviceIndex < 0 || static_cast<size_t>(m_cliOutputDeviceIndex) >= devices.size()) {
        spdlog::warn("Device index {} is out of range. Falling back to 0.", m_cliOutputDeviceIndex);
        m_cliOutputDeviceIndex = 0;
    }
    m_outputDevicesList->SetSelection(m_cliOutputDeviceIndex);
    g_Audio.selectDevice(static_cast<size_t>(m_cliOutputDeviceIndex));
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
    auto text = std::string(messageText.utf8_str());
    if (!Speech::GetInstance().speak(text.c_str())) {
        wxMessageBox("This voice either does not work with the program or crashes it. Please select another voice.",
                     "Error! The selected SAPI voice is not supported.", 5L, m_panel);
    }
    g_HistoryStorage.push(text);
    m_messageField->Clear();
}

void MainFrame::OnMessageFieldKeyDown(wxKeyEvent& event) {
    auto text = std::string(m_messageField->GetValue().utf8_str());
    switch (event.GetKeyCode()) {
        case WXK_UP:
            m_messageField->SetValue(wxString::FromUTF8(g_HistoryStorage.getPreviousByText(text)));
            break;
        case WXK_DOWN:
            m_messageField->SetValue(wxString::FromUTF8(g_HistoryStorage.getNextByText(text)));
            break;
        default:
            break;
    }
    event.Skip();
}

void MainFrame::OnVoiceChange(wxCommandEvent& event) {
    int value = m_voicesList->GetSelection();
    if (value == wxNOT_FOUND) {
        spdlog::warn("Voice selection event received with no selection");
        return;
    }
    Speech::GetInstance().setVoice(static_cast<uint64_t>(value));
}

void MainFrame::OnOutputDeviceChange(wxCommandEvent& event) {
    int value = m_outputDevicesList->GetSelection();
    if (value == wxNOT_FOUND) {
        spdlog::warn("Device selection event received with no selection");
        return;
    }
    g_Audio.selectDevice(static_cast<size_t>(value));
}

void MainFrame::OnCharEvent(wxKeyEvent& event) {
    if (event.GetKeyCode() == WXK_ESCAPE) {
        Close();
    } else {
        event.Skip();
    }
}

void MainFrame::OnHelpButton(wxCommandEvent& event) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(m_helpText));
        wxTheClipboard->Close();
    }
    wxMessageBox(m_helpText, "Help text copied to clipboard");
}

bool MyApp::OnInit() {
    CLI::App cliApp{"SIM - Speak Instead of Me speech utility"};
    auto argv = cliApp.ensure_utf8(MyApp::argv);
    bool cliIsDebugEnabled = false;
    cliApp.add_flag("-D,--debug", cliIsDebugEnabled, "Enable the debug logging for release builds");
    std::string cliVoiceName = "";
    cliApp.add_option(
        "-n,--voice-name", cliVoiceName,
        "Specify SAPI voice name to be selected at program start. If present and found, then voice index is ignored");
    int cliVoiceIndex = 0;
    cliApp.add_option("-v,--voice", cliVoiceIndex,
                      "Specify SAPI voice index to be selected at program start. If voice is selected by name and is "
                      "successfully found, then this option is ignored.");
    int cliOutputDeviceIndex = 0;
    cliApp.add_option("-d,--device", cliOutputDeviceIndex,
                      "Specify output device number to be selected at program start");
    CLI11_PARSE(cliApp, MyApp::argc, argv);

    InitializeLogging(MyApp::argc, MyApp::argv, cliIsDebugEnabled);
    auto* frame = new MainFrame(PROGRAM_TITLE, cliVoiceIndex, cliVoiceName, cliOutputDeviceIndex, cliApp.help());
    frame->Show(true);
    spdlog::debug("Main window shown");
    return true;
}

void MyApp::OnInitCmdLine(wxCmdLineParser& parser) {
    // Left empty to bypass wxWidgets cli parsing
}
