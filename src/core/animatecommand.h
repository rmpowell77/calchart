/*
 * animate.h
 * Classes for animating shows
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

#include "animate.h"

struct CC_DrawCommand;

class AnimateCommand
{
public:
	AnimateCommand(unsigned beats);
	virtual ~AnimateCommand() = default;

// returns false if end of command
	virtual bool Begin(AnimatePoint& pt);
	virtual bool End(AnimatePoint& pt);
	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

// go through all beats at once
	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() const = 0;
	virtual float RealDirection() const = 0;
	virtual float MotionDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual unsigned NumBeats() const { return mNumBeats; }

	// What style to display
	virtual MarchingStyle StepStyle() { return STYLE_HighStep; }

	// when we want to have the path drawn:
	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const;

protected:
	unsigned mNumBeats;
	unsigned mBeat;
};

class AnimateCommandStationary : public AnimateCommand
{
public:
	AnimateCommandStationary(unsigned beats, float direction);
	virtual ~AnimateCommandStationary() = default;

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
protected:
	AnimateDir dir;
	float realdir;
};

class AnimateCommandMT : public AnimateCommandStationary
{
private:
	using super = AnimateCommandStationary;
public:
	AnimateCommandMT(unsigned beats, float direction);
	virtual ~AnimateCommandMT() = default;
};

class AnimateCommandClose : public AnimateCommandStationary
{
private:
	using super = AnimateCommandStationary;
public:
	AnimateCommandClose(unsigned beats, float direction);
	virtual ~AnimateCommandClose() = default;
};

class AnimateCommandMove : public AnimateCommandMT
{
public:
	AnimateCommandMove(unsigned beats, CC_coord movement);
	AnimateCommandMove(unsigned beats, CC_coord movement, float direction);
	virtual ~AnimateCommandMove() = default;

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual float MotionDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const;

private:
	CC_coord mVector;
};

class AnimateCommandRotate : public AnimateCommand
{
public:
	AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		float ang1, float ang2, bool backwards = false);
	virtual ~AnimateCommandRotate() = default;

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual CC_DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const CC_coord& offset) const;

private:
	CC_coord mOrigin;
	float mR, mAngStart, mAngEnd;
	float mFace;
};