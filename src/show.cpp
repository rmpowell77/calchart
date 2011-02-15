/* show.cpp
 * Member functions for show classes
 *
 * Modification history:
 * 4-16-95    Garrick Meeker              Created from previous CalPrint
 * 7-28-95    Garrick Meeker              Added continuity parser from
 *                                           previous CalPrint
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

#ifdef __GNUG__
#pragma implementation
#endif

#include <wx/wx.h>
#include <wx/timer.h>
#include "show.h"
#include "undo.h"
#include "confgr.h"
#include "modes.h"
#include "calchartapp.h"
#include "cc_sheet.h"

#include "ingl.h"

#include <ctype.h>
#include <math.h>
#include <list>

CC_textline::CC_textline()
: center(false), on_main(true), on_sheet(true) {}
CC_textline::~CC_textline()
{
}


float BoundDirection(float f)
{
	while (f >= 360.0) f -= 360.0;
	while (f < 0.0) f += 360.0;
	return f;
}


float BoundDirectionSigned(float f)
{
	while (f >= 180.0) f -= 360.0;
	while (f < -180.0) f += 360.0;
	return f;
}


bool IsDiagonalDirection(float f)
{
	f = BoundDirection(f);
	return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) ||
		IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}


void CreateVector(CC_coord& c, float dir, float mag)
{
	float f;

	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		c.x = c.y = FLOAT2COORD(mag);
		if ((dir > 50.0) && (dir < 310.0)) c.x = -c.x;
		if (dir < 180.0) c.y = -c.y;
	}
	else
	{
		f = mag * cos(DEG2RAD(dir));
		c.x = FLOAT2COORD(f);
		f = mag * -sin(DEG2RAD(dir));
		c.y = FLOAT2COORD(f);
	}
}


void CreateUnitVector(float& a, float& b, float dir)
{
	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		a = b = 1.0;
		if ((dir > 50.0) && (dir < 310.0)) a = -a;
		if (dir < 180.0) b = -b;
	}
	else
	{
		a = cos(DEG2RAD(dir));
		b = -sin(DEG2RAD(dir));
	}
}


