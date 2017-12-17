/*
 * cc_continuity.cpp
 * Implementation of contiunity classes
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

#include "cc_continuity.h"
#include "cont.h"
#include <assert.h>

extern int parsecontinuity();
extern const char* yyinputbuffer;
extern std::vector<std::unique_ptr<CalChart::ContProcedure> > ParsedContinuity;

namespace CalChart {

Continuity::Continuity(std::string const& s)
    : text(s)
{
    yyinputbuffer = text.c_str();
    // parse out the error
    if (parsecontinuity() != 0) {
        ContToken dummy;
        throw ParseError(s, dummy.line, dummy.col);
    }
    m_parsedContinuity = std::move(ParsedContinuity);
}

Continuity::~Continuity() = default;

Continuity::Continuity(Continuity const& other)
    : text(other.text)
{
    for (auto&& i : other.m_parsedContinuity) {
        m_parsedContinuity.emplace_back(i->clone());
    }
}

Continuity& Continuity::operator=(Continuity const& other)
{
    Continuity copy(other);
    swap(*this, copy);
    return *this;
}

Continuity::Continuity(Continuity&&) noexcept = default;
Continuity& Continuity::operator=(Continuity&&) noexcept = default;

bool operator==(Continuity const& lhs, Continuity const& rhs)
{
    return std::equal(lhs.m_parsedContinuity.begin(), lhs.m_parsedContinuity.end(), rhs.m_parsedContinuity.begin(), rhs.m_parsedContinuity.end(), [](auto&& a, auto&& b) {
        return *a == *b;
    });
}

// Test Suite stuff
struct Continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_Continuity(const Continuity& underTest,
    const Continuity_values& values)
{
    return (underTest.text == values.text) && (underTest.GetText() == values.GetText);
}

void Continuity_UnitTests()
{
    // test some defaults:
    Continuity_values values;
    values.text = "";
    values.GetText = values.text;

    // test defaults
    Continuity underTest;
    assert(Check_Continuity(underTest, values));

    // test defaults with different init
    Continuity underTest2;
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Set some text
    underTest2 = Continuity{ "mt E REM" };
    values.text = "mt E REM";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Set some text
    underTest2 = Continuity{ "ewns np" };
    values.text = "ewns np";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Reset text
    underTest2 = Continuity{ "" };
    values.text = "";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));
}
}
