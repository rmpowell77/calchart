/* show.h
 * Definitions for the show classes
 *
 * Modification history:
 * 1-2-95     Garrick Meeker              Created from previous CalPrint
 * 4-16-95    Garrick Meeker              Converted to C++
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#ifndef _CC_CONTINUITY_H_
#define _CC_CONTINUITY_H_

#ifdef __GNUG__
#pragma interface
#endif

#include <wx/wx.h>
#include <boost/shared_ptr.hpp>

class CC_continuity
{
public:
	CC_continuity(const wxString& s, unsigned n);
	~CC_continuity();

	const wxString& GetName() const;
	unsigned GetNum() const;

	void SetText(const wxString& s);
	void AppendText(const wxString& s);
	const wxString& GetText() const;

private:
	wxString name;
	unsigned num;
	wxString text;

friend bool Check_CC_continuity(const CC_continuity&, const struct CC_continuity_values&);
};

typedef boost::shared_ptr<CC_continuity> CC_continuity_ptr;

bool Check_CC_continuity(const CC_continuity&, const struct CC_continuity_values&);

void CC_continuity_UnitTests();

#endif
