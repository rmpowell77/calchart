/*
 * modes.cpp
 * Handle show mode classes
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

#include "modes.h"

#include <algorithm>

namespace CalChart {

static constexpr const char* kYardTextDefaults[] = {
    "N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D", "C", "B", "A", "-10", "-5",
    "0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50",
    "45", "40", "35", "30", "25", "20", "15", "10", "5", "0",
    "-5", "-10", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"
};

ShowMode::YardLinesInfo_t ShowMode::GetDefaultYardLines() {
    ShowMode::YardLinesInfo_t data;
    for (auto i = 0; i < kYardTextValues; ++i) {
        data[i] = kYardTextDefaults[i];
    }
    return data;
}

ShowMode::ShowMode(CalChart::Coord size,
    CalChart::Coord offset,
    CalChart::Coord border1,
    CalChart::Coord border2,
    unsigned short whash,
    unsigned short ehash,
    YardLinesInfo_t const& yardlines)
    : mSize(size)
    , mOffset(offset)
    , mBorder1(border1)
    , mBorder2(border2)
    , mHashW(whash)
    , mHashE(ehash)
    , mYardLines(yardlines)
{
}

CalChart::Coord ShowMode::ClipPosition(const CalChart::Coord& pos) const
{
    auto min = MinPosition();
    auto max = MaxPosition();

    CalChart::Coord clipped;
    if (pos.x < min.x)
        clipped.x = min.x;
    else if (pos.x > max.x)
        clipped.x = max.x;
    else
        clipped.x = pos.x;
    if (pos.y < min.y)
        clipped.y = min.y;
    else if (pos.y > max.y)
        clipped.y = max.y;
    else
        clipped.y = pos.y;
    return clipped;
}

ShowMode::ShowModeInfo_t
ShowMode::GetShowModeInfo() const
{
    return { {
        mHashW,
        mHashE,
        CoordUnits2Int(mBorder1.x),
        CoordUnits2Int(mBorder1.y),
        CoordUnits2Int(mBorder2.x),
        CoordUnits2Int(mBorder2.y),
        CoordUnits2Int(-mOffset.x),
        CoordUnits2Int(-mOffset.y),
        CoordUnits2Int(mSize.x),
        CoordUnits2Int(mSize.y),
    } };
}

ShowMode
ShowMode::CreateShowMode(ShowModeInfo_t const& values, YardLinesInfo_t const& yardlines)
{
    unsigned short whash = values[kwhash];
    unsigned short ehash = values[kehash];
    CalChart::Coord bord1{ Int2CoordUnits(values[kbord1_x]), Int2CoordUnits(values[kbord1_y]) };
    CalChart::Coord bord2{ Int2CoordUnits(values[kbord2_x]), Int2CoordUnits(values[kbord2_y]) };
    CalChart::Coord offset{ Int2CoordUnits(-values[koffset_x]), Int2CoordUnits(-values[koffset_y]) };
    CalChart::Coord size{ Int2CoordUnits(values[ksize_x]), Int2CoordUnits(values[ksize_y]) };
    return {
        size,
        offset,
        bord1,
        bord2,
        whash,
        ehash,
        yardlines
    };
}

ShowMode ShowMode::CreateShowMode(CalChart::Coord size, CalChart::Coord offset, CalChart::Coord border1,
    CalChart::Coord border2, unsigned short whash, unsigned short ehash, YardLinesInfo_t const& yardlines)
{
    return {
        size,
        offset,
        border1,
        border2,
        whash,
        ehash,
        yardlines
    };
}

    ShowMode::YardLinesInfo_t const& ShowMode::Get_yard_text() const
{
    return mYardLines;
}


ShowMode ShowMode::GetDefaultShowMode()
{
    return ShowMode::CreateShowMode({ { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } }, GetDefaultYardLines());
}

}
