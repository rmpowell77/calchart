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
extern std::vector<std::unique_ptr<ContProcedure> > ParsedContinuity;

CC_continuity::CC_continuity(std::string const& s)
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

CC_continuity::~CC_continuity() = default;

CC_continuity::CC_continuity(CC_continuity const& other)
    : text(other.text)
{
    for (auto&& i : other.m_parsedContinuity) {
        m_parsedContinuity.emplace_back(i->clone());
    }
}

CC_continuity& CC_continuity::operator=(CC_continuity const& other)
{
    CC_continuity copy(other);
    swap(*this, copy);
    return *this;
}

CC_continuity::CC_continuity(CC_continuity&&) noexcept = default;
CC_continuity& CC_continuity::operator=(CC_continuity&&) noexcept = default;

// Test Suite stuff
struct CC_continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_CC_continuity(const CC_continuity& underTest,
    const CC_continuity_values& values)
{
    return (underTest.text == values.text) && (underTest.GetText() == values.GetText);
}

void CC_continuity_UnitTests()
{
    // test some defaults:
    CC_continuity_values values;
    values.text = "";
    values.GetText = values.text;

    // test defaults
    CC_continuity underTest;
    assert(Check_CC_continuity(underTest, values));

    // test defaults with different init
    CC_continuity underTest2;
    values.GetText = values.text;
    assert(Check_CC_continuity(underTest2, values));

    // Set some text
    underTest2 = CC_continuity{ "mt E REM" };
    values.text = "mt E REM";
    values.GetText = values.text;
    assert(Check_CC_continuity(underTest2, values));

    // Set some text
    underTest2 = CC_continuity{ "ewns np" };
    values.text = "ewns np";
    values.GetText = values.text;
    assert(Check_CC_continuity(underTest2, values));

    // Reset text
    underTest2 = CC_continuity{ "" };
    values.text = "";
    values.GetText = values.text;
    assert(Check_CC_continuity(underTest2, values));
}
