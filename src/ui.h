#pragma once

#include <wx/event.h>
#include <wx/wx.h>

class MainFrame : public wxFrame {
  public:
    MainFrame(const wxString& title, int cliVoiceIndex = 0, std::string cliVoiceName = "", int cliOutputDeviceIndex = 0,
              std::string cliHelpText = "");

  private:
    wxPanel* m_panel;
    wxTextCtrl* m_messageField;
    wxListBox* m_voicesList;
    wxListBox* m_outputDevicesList;
    wxSlider* m_rateSlider;
    wxSlider* m_volumeSlider;
    wxButton* m_helpButton;
    int m_cliVoiceIndex = 0;
    std::string m_cliVoiceName;
    int m_cliOutputDeviceIndex = 0;
    std::string m_helpText;

    void populateVoicesList();
    void populateDevicesList();
    void OnEnterPress(wxCommandEvent& event);
    void OnMessageFieldKeyDown(wxKeyEvent& event);
    void OnVoiceChange(wxCommandEvent& event);
    void OnOutputDeviceChange(wxCommandEvent& event);
    void OnRateSliderChange(wxCommandEvent& event);
    void OnVolumeSliderChange(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnCharEvent(wxKeyEvent& event);
    void OnHelpButton(wxCommandEvent& event);
};

class MyApp : public wxApp {
  public:
    virtual bool OnInit() override;
    virtual void OnInitCmdLine(wxCmdLineParser& parser) override;
};
