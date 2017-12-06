/*
 * cc_continuity.h
 * Definitions for the continuity classes
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

#include <string>
#include <list>

class ContProcedure;
class ContinuityEditor;

struct ParseError : public std::runtime_error
{
    ParseError(int l, int c) : std::runtime_error("ParseError"), line(l), column(c) {}
    int line, column;
};

class CC_continuity {
public:
    CC_continuity(std::string const& s = "");
    ~CC_continuity();

//    CC_continuity(CC_continuity const&);
//    CC_continuity& operator=(CC_continuity const&);
//    CC_continuity(CC_continuity&&);
//    CC_continuity& operator=(CC_continuity&&);

// this could throw
    std::vector<std::unique_ptr<ContProcedure> > GetParsedContinuity() const;

private:
    const std::string& GetText() const;

    std::string text;
//    std::list<std::unique_ptr<ContProcedure> > m_parsedContinuity;

    friend ContinuityEditor;
    friend class AnimationFrame;
    friend class CC_sheet;
    friend class CC_show;
    friend void DumpContinuity(const char* show);


    friend bool Check_CC_continuity(const CC_continuity&,
        const struct CC_continuity_values&);
    friend void CC_continuity_UnitTests();
};

bool Check_CC_continuity(const CC_continuity&,
    const struct CC_continuity_values&);

void CC_continuity_UnitTests();
