#pragma once
/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include <wx/wx.h>

namespace CalChart {
class ContProcedure;
}

class ContComposerPanel;

class ContComposerDialog : public wxDialog {
    using super = wxDialog;
    DECLARE_CLASS(ContComposerDialog)
    DECLARE_EVENT_TABLE()

public:
    ContComposerDialog(std::unique_ptr<CalChart::ContProcedure> starting_continuity,
        wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Compose Continuity"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxDEFAULT_DIALOG_STYLE,
        const wxString& name = wxDialogNameStr);
    ~ContComposerDialog();

    std::unique_ptr<CalChart::ContProcedure> GetContinuity();
    virtual bool Validate() override;

private:
    ContComposerPanel* mPanel;
};
