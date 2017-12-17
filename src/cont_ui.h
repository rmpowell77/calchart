/*
 * cont_ui.h
 * Header for continuity editors
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

#pragma once

#include "calchartdoc.h"
#include "cont_ui_cell.h"

#include <wx/docview.h>
#include <wx/dialog.h>

class ContinuityEditor;
class FancyTextWin;
class ContinuityEditorCanvas;
class ContinuityEditorPerCont;
class ContinuityEditorView;

class ContinuityEditorCanvas : public CustomListViewPanel {
    using super = CustomListViewPanel;

public:
    // Basic functions
    ContinuityEditorCanvas(CalChartDoc* doc, SYMBOL_TYPE sym, CalChartConfiguration& config, wxWindow* parent);
    virtual ~ContinuityEditorCanvas() = default;
    void DoSetContinuity(CalChart::Continuity const& new_cont);

private:
    virtual void OnNewEntry(int cell) override;
    virtual void OnDeleteEntry(int cell) override;
    virtual void OnMoveEntry(int start_cell, int end_cell) override;

    void DoSetFocus(wxFocusEvent& event);
    void DoKillFocus(wxFocusEvent& event);

    void UpdateCont(CalChart::Continuity const& new_cont);

    CalChartDoc* mDoc;
    CalChart::Continuity mCont;
    SYMBOL_TYPE mSym;
    CalChartConfiguration& mConfig;
    DECLARE_EVENT_TABLE()
};

// ContinuityEditor
// The way you edit the continuity for individual marchers
// This dialog should notify the user to save if there are any outstanding
// edits.

class ContinuityEditor : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    ContinuityEditor(CalChartDoc* dcr, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize);
    ~ContinuityEditor();

    void OnCmdHelp(wxCommandEvent& event);
    void Update(); // Refresh all window controls

private:
    void CreateControls();

    void OnChar(wxKeyEvent& event);
    SYMBOL_TYPE CurrentSymbolChoice() const;

    void Save();
    void Discard();
    void SetCurrent(unsigned i);

    CalChartDoc* mDoc;
    std::unique_ptr<ContinuityEditorView> mView;
    std::vector<ContinuityEditorPerCont*> mPerCont;

    DECLARE_EVENT_TABLE()
};
