/*
 * print_ps_dialog.h
 * Dialox box for printing postscript
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "platconf.h"

#include <set>
#include <wx/dialog.h>
#include <wx/wx.h>

class CalChartDoc;
class CalChartConfiguration;

class PrintPostScriptDialog : public wxDialog {
    DECLARE_CLASS(PrintPostScriptDialog)
    DECLARE_EVENT_TABLE()

public:
    PrintPostScriptDialog();
    PrintPostScriptDialog(const CalChartDoc* doc, wxFrame* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Print Dialog"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~PrintPostScriptDialog();

    void Init();

    bool Create(const CalChartDoc* show, wxFrame* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Print Dialog"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

    void CreateControls();

    void ShowPrintSelect(wxCommandEvent&);
    // because we modify setting, we need some way to reset them
    void ResetDefaults(wxCommandEvent&);

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    // to print a show, call this function
    void PrintShow(const CalChartConfiguration& config);

private:
    const CalChartDoc* mShow;
    wxTextCtrl* text_cmd;
#ifdef PRINT__RUN_CMD
    wxTextCtrl *text_opts, *text_view_cmd, *text_view_opts;
#endif
    wxTextCtrl *text_x, *text_y, *text_width, *text_height, *text_length;
    wxTextCtrl* text_minyards;
    wxRadioBox *radio_orient, *radio_method;
    wxCheckBox *check_cont, *check_pages, *check_overview;
    std::set<size_t> mIsSheetPicked;
};
