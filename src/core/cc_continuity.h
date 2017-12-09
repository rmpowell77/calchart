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
#include <vector>

class ContProcedure;
class ContinuityEditor;

struct ParseError : public std::runtime_error {
    ParseError(std::string const& str, int l, int c)
        : std::runtime_error(std::string("ParseError of ") + str + " at " + std::to_string(l) + ", " + std::to_string(c))
        , line(l)
        , column(c)
    {
    }
    int line, column;
};

class CC_continuity {
public:
    // this could throw ParseError
    CC_continuity(std::string const& s = "");
    ~CC_continuity();

    CC_continuity(CC_continuity const&);
    CC_continuity& operator=(CC_continuity const&);
    CC_continuity(CC_continuity&&) noexcept;
    CC_continuity& operator=(CC_continuity&&) noexcept;

    friend void swap(CC_continuity& lhs, CC_continuity& rhs)
    {
        using std::swap;
        swap(lhs.text, rhs.text);
        swap(lhs.m_parsedContinuity, rhs.m_parsedContinuity);
    }

    auto GetText() const noexcept { return text; }

    std::vector<std::unique_ptr<ContProcedure> > const& GetParsedContinuity() const noexcept { return m_parsedContinuity; }

private:
    std::string text;
    std::vector<std::unique_ptr<ContProcedure> > m_parsedContinuity;

    friend bool Check_CC_continuity(const CC_continuity&, const struct CC_continuity_values&);
    friend void CC_continuity_UnitTests();
};

bool Check_CC_continuity(const CC_continuity&,
    const struct CC_continuity_values&);

void CC_continuity_UnitTests();
