#pragma once
/*
 * cont_ui_cell.h
 * Header for continuity editor cell
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

#include "custom_listview.h"

class CalChartConfiguration;
namespace CalChart {
class ContProcedure;
}

class ContinuityEditorCell : public CustomListViewCell {
public:
    ContinuityEditorCell(CalChart::ContProcedure& proc, CalChartConfiguration const& config);
    virtual ~ContinuityEditorCell() = default;
    virtual void DrawToDC(wxDC& dc) override;
    virtual int Height() const override;
    virtual int Width() const override;

private:
    CalChart::ContProcedure& mProc;
    CalChartConfiguration const& mConfig;
};
