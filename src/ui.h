#pragma once

#include <wx/event.h>
#include <wx/wx.h>

class MainFrame : public wxFrame {
  public:
    MainFrame(const wxString& title);

  private:
    wxPanel* m_panel;
    wxTextCtrl* m_messageField;
    wxListBox* m_voicesList;
    wxListBox* m_outputDevicesList;
    wxSlider* m_rateSlider;
    wxSlider* m_volumeSlider;

    void populateVoicesList();
    void populateDevicesList();
    void OnEnterPress(wxCommandEvent& event);
    void OnVoiceChange(wxCommandEvent& event);
    void OnOutputDeviceChange(wxCommandEvent& event);
    void OnRateSliderChange(wxCommandEvent& event);
    void OnVolumeSliderChange(wxCommandEvent& event);
    void OnRefresh(wxCommandEvent& event);
    void OnCharEvent(wxKeyEvent& event);
};

class MyApp : public wxApp {
  public:
    virtual bool OnInit();
};
