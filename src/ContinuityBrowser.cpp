/*
 * ContinuityBrowser
 * Continuity editors
 */

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

#include "ContinuityBrowser.h"
#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "CalChartView.h"
#include "ContinuityBrowserPanel.h"
#include "cc_sheet.h"
#include "confgr.h"

#include <wx/help.h>
#include <wx/statline.h>

// a panel consists of the name, canvas
class ContinuityBrowserPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityBrowserPerCont(SYMBOL_TYPE sym, wxWindow* parent);
    virtual ~ContinuityBrowserPerCont() = default;

    void DoSetContinuity(CalChart::Continuity const& new_cont);

    void SetView(CalChartView* view);
    auto GetView() const { return mView; }

private:
    CalChartView* mView;
    ContinuityBrowserPanel* mCanvas;
};

ContinuityBrowserPerCont::ContinuityBrowserPerCont(SYMBOL_TYPE sym, wxWindow* parent)
    : wxPanel(parent)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto staticText = new wxStaticText(this, wxID_STATIC, CalChart::GetLongNameForSymbol(sym), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    topsizer->Add(staticText, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    // here's a canvas
    mCanvas = new ContinuityBrowserPanel(sym, CalChartConfiguration::GetGlobalConfig(), this);
    topsizer->Add(mCanvas, 1, wxEXPAND);
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);
}

void ContinuityBrowserPerCont::DoSetContinuity(CalChart::Continuity const& new_cont)
{
    mCanvas->DoSetContinuity(new_cont);
}

void ContinuityBrowserPerCont::SetView(CalChartView* view)
{
    mView = view;
    mCanvas->SetView(view);
}

ContinuityBrowser::ContinuityBrowser(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
{
    // create a sizer for laying things out top down:
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add a horizontal bar to make things clear:
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // we lay things out from top to bottom, saying what point we're dealing with, then the continuity
    for (auto&& eachcont : k_symbols) {
        mPerCont.push_back(new ContinuityBrowserPerCont(eachcont, this));
        topsizer->Add(mPerCont.back(), 0, wxGROW | wxALL, 5);
        mPerCont.back()->Show(false);
    }

    // add help
    auto top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(top_button_sizer);
    auto button = new wxButton(this, wxID_HELP, wxT("&Help"));
    top_button_sizer->Add(button, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    button->Bind(wxEVT_BUTTON, [](auto const&) {
        wxGetApp().GetGlobalHelpController().LoadFile();
        wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
    });

    SetScrollRate(1, 1);

    // now update the current screen
    Update();
}

ContinuityBrowser::~ContinuityBrowser() = default;

void ContinuityBrowser::OnUpdate()
{
    auto sht = mView->GetCurrentSheet();

    for (auto i = 0ul; i < sizeof(k_symbols) / sizeof(k_symbols[0]); ++i) {
        mPerCont.at(i)->Show(sht->ContinuityInUse(k_symbols[i]));
        mPerCont.at(i)->DoSetContinuity(sht->GetContinuityBySymbol(k_symbols[i]));
    }
    GetSizer()->FitInside(this);
}

void ContinuityBrowser::SetView(CalChartView* view)
{
    mView = view;
    for (auto&& cont : mPerCont) {
        cont->SetView(view);
    }
}
