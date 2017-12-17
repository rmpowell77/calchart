/*
 * custom_listview.h
 * Header for basic wxWindows classes
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

#include <wx/wx.h>
#include <set>
#include <vector>

// the idea is that we will have a collection of smaller cells, that can be moved around.
// View for linking CalChartDoc with ContinuityEditor
class CustomListViewCell {
public:
    CustomListViewCell(){};
    virtual ~CustomListViewCell() = default;
    virtual void DrawToDC(wxDC&) = 0;
    virtual int Height() const = 0;
    virtual int Width() const = 0;
};

// the idea is that we will have a collection of smaller cells, that can be moved around.
// View for linking CalChartDoc with ContinuityEditor
class CustomListViewPanel : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    // Basic functions
    CustomListViewPanel(wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize);
    virtual ~CustomListViewPanel() = default;
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnLeftDownMouseEvent(wxMouseEvent& event);
    void OnLeftUpMouseEvent(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void SetSelection(int which) { m_selected = static_cast<size_t>(which); }
    auto GetSelection() const { return m_selected; }
    void SetContCells(std::vector<std::unique_ptr<CustomListViewCell> > cells);

private:
    virtual void OnNewEntry(int cell);
    virtual void OnDeleteEntry(int cell);
    virtual void OnMoveEntry(int start_cell, int end_cell);
    size_t WhichCell(wxPoint const& point) const;

    std::vector<std::unique_ptr<CustomListViewCell> > mContCells;
    wxPoint m_firstPress;
    wxPoint m_lastLocation;
    size_t m_selected;
    bool m_dragging;

    DECLARE_EVENT_TABLE()
};
