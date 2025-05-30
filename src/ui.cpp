#include "ui.h"

#include "loggerSetup.h"
#include "speech.h"

#include <spdlog/spdlog.h>

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title) {
    m_panel = new wxPanel(this, wxID_ANY);
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);
    auto* selectionsSizer = new wxBoxSizer(wxHORIZONTAL);
    m_messageField = new wxTextCtrl(m_panel, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                                    wxTE_DONTWRAP | wxTE_PROCESS_ENTER);
    m_voicesList = new wxListBox(m_panel, wxID_ANY);
    m_outputDevicesList = new wxListBox(m_panel, wxID_ANY);
    selectionsSizer->Add(m_voicesList);
    selectionsSizer->Add(m_outputDevicesList);
    mainSizer->Add(m_messageField);
    mainSizer->Add(selectionsSizer);
    m_messageField->SetFocus();
    m_panel->SetSizer(mainSizer);
    Layout();
    populateVoicesList();
    populateDevicesList();
}

void MainFrame::populateVoicesList() {
    m_voicesList->Clear();
    m_voicesList->AppendString("Voice number one");
    m_voicesList->AppendString("Voice number zwei!");
    m_voicesList->SetSelection(0);
}

void MainFrame::populateDevicesList() {
    m_outputDevicesList->Clear();
    m_outputDevicesList->AppendString("Default device");
    m_outputDevicesList->AppendString("Some other device");
    m_outputDevicesList->SetSelection(0);
}

bool MyApp::OnInit() {
    InitializeLogging(MyApp::argc, MyApp::argv);
    if (!wxApp::OnInit()) {
        spdlog::critical("Failed to initialize WX");
        return false;
    }
    MainFrame* frame = new MainFrame("SIM test");
    frame->Show(true);
    spdlog::debug("Main windo shown");
    return true;
}
