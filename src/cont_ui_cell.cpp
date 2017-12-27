/*
 * cont_ui_cell.cpp
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

#include "cont_ui_cell.h"

#include "basic_ui.h"
#include "cont_ui.h"
#include "confgr.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "confgr.h"
#include "cc_command.h"
#include "calchartapp.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include "animate.h"
#include "animatecompile.h"
#include "cont.h"
#include "custom_listview.h"

#include <regex>

static int GetTextBoxSize(wxDC const& dc, CalChartConfiguration const& config, CalChart::DrawableCont const& drawCont, int box_padding, int text_padding)
{
    auto total_size = 0;

    const static auto format_sub_size = dc.GetTextExtent("%@").x;
    for (auto&& i : drawCont.args) {
        total_size += GetTextBoxSize(dc, config, i, box_padding, text_padding) + text_padding * 2;
    }
    total_size += dc.GetTextExtent(drawCont.description).x;
    total_size -= format_sub_size * drawCont.args.size();

    return total_size;
}

auto GetBrush(CalChart::ContType contType, CalChartConfiguration const& config)
{
    switch (contType) {
    case CalChart::ContType::procedure:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_PROC).first;
    case CalChart::ContType::value:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_VALUE).first;
    case CalChart::ContType::function:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_FUNCTION).first;
    case CalChart::ContType::direction:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_DIRECTION).first;
    case CalChart::ContType::steptype:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_STEPTYPE).first;
    case CalChart::ContType::point:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_POINT).first;
    }
    return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_PROC).first;
}

static void DrawProcCellBox(wxDC& dc, CalChartConfiguration const& config, CalChart::DrawableCont const& drawCont, int box_size, int box_padding, int text_padding, int x_start, int y_start, int rounding)
{
    // first draw the box of the total size
    auto fullbox_size = GetTextBoxSize(dc, config, drawCont, box_padding, text_padding) + 2 * text_padding;
    auto&& current_brush = dc.GetBrush();
    // fill with the color for this continuity
    dc.SetBrush(GetBrush(drawCont.type, config));
    dc.DrawRoundedRectangle(x_start, y_start, fullbox_size, box_size, rounding);
    dc.SetBrush(current_brush);

    // then render everything from left to right, text, then descend, then text...
    const static std::regex format_substr("%@");
    std::smatch format_match;
    auto string_to_print = drawCont.description;
    auto count = 0u;
    const auto text_top = y_start + text_padding;
    auto text_left = x_start + text_padding;
    while (regex_search(string_to_print, format_match, format_substr)) {
        auto t_str_to_print = wxString(format_match.prefix());
        auto text_size = dc.GetTextExtent(t_str_to_print).x;
        //        std::cout<<"found prefix "<<format_match.prefix()<<" suffix "<<format_match.suffix()<<"\n";
        dc.DrawText(t_str_to_print, text_left, text_top);
        text_left += text_size;
        DrawProcCellBox(dc, config, drawCont.args.at(count), box_size, box_padding, text_padding, text_left, y_start, rounding);
        text_left += GetTextBoxSize(dc, config, drawCont.args.at(count), box_padding, text_padding) + 2 * text_padding;
        ++count;
        string_to_print = format_match.suffix();
    }
    dc.DrawText(string_to_print, text_left, text_top);
}

static void DrawProcCell(wxDC& dc, CalChartConfiguration const& config, CalChart::DrawableCont const& drawCont, int cell_size, int box_padding, int text_padding, int x_start, int y_start, int rounding)
{
    DrawProcCellBox(dc, config, drawCont, cell_size - 2 * box_padding, box_padding, text_padding, x_start + box_padding, y_start + box_padding, rounding);
}

ContinuityEditorCell::ContinuityEditorCell(CalChart::ContProcedure& proc, CalChartConfiguration const& config)
    : mProc(proc)
    , mConfig(config)
{
}

void ContinuityEditorCell::DrawToDC(wxDC& dc)
{
    wxFont* contPlainFont = wxTheFontList->FindOrCreateFont(mConfig.Get_ContCellFontSize(), wxFONTFAMILY_SWISS, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
    dc.SetFont(*contPlainFont);

    DrawProcCell(dc, mConfig, mProc.GetDrawableCont(), Height(), mConfig.Get_ContCellBoxPadding(), mConfig.Get_ContCellTextPadding(), 0, 0, mConfig.Get_ContCellRounding());
}

int ContinuityEditorCell::Height() const
{
    return mConfig.Get_ContCellFontSize() + 2 * mConfig.Get_ContCellBoxPadding() + 2 * mConfig.Get_ContCellTextPadding();
}

int ContinuityEditorCell::Width() const
{
    wxMemoryDC temp_dc;
    return GetTextBoxSize(temp_dc, mConfig, mProc.GetDrawableCont(), mConfig.Get_ContCellBoxPadding(), mConfig.Get_ContCellTextPadding()) + 2 * mConfig.Get_ContCellBoxPadding() + 2 * mConfig.Get_ContCellTextPadding();
}
