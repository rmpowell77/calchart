/*
 * cont_ui.cpp
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

#include "basic_ui.h"
#include "cont_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include "animate.h"
#include "animatecompile.h"
#include "cont.h"
#include "custom_listview.h"
#include "cont_composer.h"

#include <regex>

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/statline.h>
#include <wx/msgdlg.h>
#include <wx/dcbuffer.h>

using namespace CalChart;

BEGIN_EVENT_TABLE(ContinuityEditorCanvas, CustomListViewPanel)
EVT_SET_FOCUS(ContinuityEditorCanvas::DoSetFocus)
EVT_KILL_FOCUS(ContinuityEditorCanvas::DoKillFocus)
END_EVENT_TABLE()

// Define a constructor for field canvas
ContinuityEditorCanvas::ContinuityEditorCanvas(CalChartDoc* doc, SYMBOL_TYPE sym, CalChartConfiguration& config, wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, winid, pos, size, style, name)
    , mDoc(doc)
    , mSym(sym)
    , mConfig(config)
{
    // make room for about 5
    auto current_size = GetMinSize();
    current_size.y = ContinuityBoxDrawer::GetHeight(config);
    current_size.y *= 5;
    SetMinSize(current_size);
}

void ContinuityEditorCanvas::DoSetContinuity(CalChart::Continuity const& text)
{
    std::vector<std::unique_ptr<DrawableCell> > contCells;
    mCont = text;
    for (auto&& i : mCont.GetParsedContinuity()) {
        contCells.emplace_back(std::make_unique<ContinuityBoxDrawer>(i->GetDrawableCont(), mConfig));
    }
    SetCells(std::move(contCells));
    Refresh();
}

template <typename T>
static auto do_cloning(T const& cont)
{
    std::vector<std::unique_ptr<ContProcedure> > copied_cont;
    for (auto&& i : cont.GetParsedContinuity()) {
        copied_cont.emplace_back(i->clone());
    }
    return copied_cont;
}

void ContinuityEditorCanvas::OnNewEntry(int cell)
{
    if (!mDoc)
        return;
    ContComposerDialog dialog(nullptr, this);

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    try {
        // if we have -1, that means push_back.
        auto copied_cont = do_cloning(mCont);
        copied_cont.insert(copied_cont.begin() + cell, dialog.GetContinuity());
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    }
    catch (std::runtime_error const& e) {
        wxMessageBox(wxT("Error: ") + wxString{ e.what() }, wxT("Parsing Error"), wxOK | wxICON_INFORMATION, this);
        return;
    }
}

void ContinuityEditorCanvas::OnEditEntry(int cell)
{
    if (!mDoc)
        return;
    ContComposerDialog dialog(mCont.GetParsedContinuity().at(cell)->clone(), this);

    if (dialog.ShowModal() != wxID_OK) {
        return;
    }
    try {
        // if we have -1, that means push_back.
        auto copied_cont = do_cloning(mCont);
        copied_cont.at(cell) = dialog.GetContinuity();
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    }
    catch (std::runtime_error const& e) {
        wxMessageBox(wxT("Error: ") + wxString{ e.what() }, wxT("Parsing Error"), wxOK | wxICON_INFORMATION, this);
        return;
    }
}

void ContinuityEditorCanvas::OnDeleteEntry(int cell)
{
    if (!mDoc)
        return;
    // make a copy, then delete it, then set as new continuity:
    if (cell < static_cast<int>(mCont.GetParsedContinuity().size())) {
        auto copied_cont = do_cloning(mCont);
        copied_cont.erase(copied_cont.begin() + cell);
        UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
    }
}

void ContinuityEditorCanvas::OnMoveEntry(int start_cell, int end_cell)
{
    auto copied_cont = do_cloning(mCont);
    if (start_cell < end_cell) {
        auto end = end_cell >= static_cast<int>(copied_cont.size()) ? copied_cont.end() : copied_cont.begin() + end_cell + 1;
        std::rotate(copied_cont.begin() + start_cell, copied_cont.begin() + start_cell + 1, end);
    }
    else {
        auto end = copied_cont.begin() + start_cell + 1;
        std::rotate(copied_cont.begin() + end_cell, copied_cont.begin() + start_cell, end);
    }
    UpdateCont(CalChart::Continuity{ std::move(copied_cont) });
}

void ContinuityEditorCanvas::UpdateCont(Continuity const& new_cont)
{
    if (!mDoc)
        return;
    auto cmd = mDoc->Create_SetContinuityCommand(mSym, new_cont);
    mDoc->GetCommandProcessor()->Submit(cmd.release());
}

void ContinuityEditorCanvas::DoSetFocus(wxFocusEvent& event)
{
    if (!mDoc)
        return;
    auto&& sht = mDoc->GetCurrentSheet();
    mDoc->SetSelection(sht->MakeSelectPointsBySymbol(mSym));
}

void ContinuityEditorCanvas::DoKillFocus(wxFocusEvent& event)
{
    SetSelection(-1);
}

// a panel consists of the name, canvas
class ContinuityEditorPerCont : public wxPanel {
    using super = wxPanel;

public:
    ContinuityEditorPerCont(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent);
    virtual ~ContinuityEditorPerCont() = default;

    void DoSetContinuity(Continuity const& new_cont);

private:
    ContinuityEditorCanvas* mCanvas;
};

ContinuityEditorPerCont::ContinuityEditorPerCont(CalChartDoc* doc, SYMBOL_TYPE sym, wxWindow* parent)
    : wxPanel(parent)
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    auto staticText = new wxStaticText(this, wxID_STATIC, GetLongNameForSymbol(sym), wxDefaultPosition, wxDefaultSize, wxALIGN_LEFT);
    topsizer->Add(staticText, 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    // here's a canvas
    mCanvas = new ContinuityEditorCanvas(doc, sym, CalChartConfiguration::GetGlobalConfig(), this);
    topsizer->Add(mCanvas, 1, wxEXPAND);
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);
}

void ContinuityEditorPerCont::DoSetContinuity(CalChart::Continuity const& text)
{
    mCanvas->DoSetContinuity(text);
}

// View for linking CalChartDoc with ContinuityEditor
class ContinuityEditorView : public wxView {
public:
    ContinuityEditorView() = default;
    virtual ~ContinuityEditorView() = default;
    virtual void OnDraw(wxDC* dc) {}
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
};

void ContinuityEditorView::OnUpdate(wxView* sender, wxObject* hint)
{
    dynamic_cast<ContinuityEditor*>(GetFrame())->Update();
}

BEGIN_EVENT_TABLE(ContinuityEditor, wxScrolledWindow)
EVT_BUTTON(wxID_HELP, ContinuityEditor::OnCmdHelp)
END_EVENT_TABLE()

ContinuityEditor::ContinuityEditor(CalChartDoc* doc, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : wxScrolledWindow(parent, id, pos, size, style, name)
    , mDoc(doc)
{
    mView = std::make_unique<ContinuityEditorView>();
    mView->SetDocument(doc);
    mView->SetFrame(this);

    // create a sizer for laying things out top down:
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    // add a horizontal bar to make things clear:
    topsizer->Add(new wxStaticLine(this, wxID_STATIC, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL), 0, wxGROW | wxALL, 5);

    // we lay things out from top to bottom, saying what point we're dealing with, then the continuity
    for (auto& eachcont : k_symbols) {
        mPerCont.push_back(new ContinuityEditorPerCont(doc, eachcont, this));
        topsizer->Add(mPerCont.back(), 0, wxGROW | wxALL, 5);
        mPerCont.back()->Show(false);
    }

    // add a save, discard, close, and help
    auto top_button_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_button_sizer->Add(new wxButton(this, wxID_HELP, wxT("&Help")), 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    topsizer->Add(top_button_sizer);

    SetScrollRate(5, 5);
    // now update the current screen
    Update();
}

ContinuityEditor::~ContinuityEditor() = default;

void ContinuityEditor::OnCmdHelp(wxCommandEvent& event)
{
    wxGetApp().GetGlobalHelpController().LoadFile();
    wxGetApp().GetGlobalHelpController().KeywordSearch(wxT("Animation Commands"));
}

void ContinuityEditor::Update()
{
    auto sht = mDoc->GetCurrentSheet();

    for (auto i = 0ul; i < sizeof(k_symbols) / sizeof(k_symbols[0]); ++i) {
        mPerCont.at(i)->Show(sht->ContinuityInUse(k_symbols[i]));
        mPerCont.at(i)->DoSetContinuity(sht->GetContinuityBySymbol(k_symbols[i]));
    }
    GetSizer()->FitInside(this);
}

void ContinuityEditor::OnChar(wxKeyEvent& event)
{
    event.Skip();
}
