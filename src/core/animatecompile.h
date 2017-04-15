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

#include "animate_types.h"
#include "animate.h"
#include "cc_show.h"

#include <vector>
#include <array>
#include <memory>
#include <list>

class ContProcedure;
class ContToken;
class AnimateCommand;

using AnimationVariables = std::array<std::map<unsigned, float>, NUMCONTVARS>;

using AnimateCommands = std::vector<std::shared_ptr<AnimateCommand> >;

class AnimationErrors {
public:
    auto AnyErrors() const { return !mErrorMarkers.empty(); }
    auto GetErrors() const { return mErrorMarkers; }
    void RegisterError(AnimateError err, const ContToken* token, unsigned curr_pt,
        SYMBOL_TYPE contsymbol);

private:
    std::map<AnimateError, ErrorMarker> mErrorMarkers;
};

struct AnimateState {
    AnimatePoint pt{};
    unsigned beats_rem{};
    AnimationVariables mVars{};
    AnimationErrors error_markers{};
    AnimateCommands cmds{};
};

class AnimateCompile {
public:
    // Compile a point
    static std::tuple<AnimateCommands, AnimationVariables, AnimationErrors>
    Compile(const CC_show& show, AnimationVariables variablesStates,
        AnimationErrors errors, CC_show::const_CC_sheet_iterator_t c_sheet,
        unsigned pt_num, SYMBOL_TYPE cont_symbol,
        std::list<std::unique_ptr<ContProcedure> > const& proc);

private:
    AnimateCompile(const CC_show& show, SYMBOL_TYPE cont_symbol, unsigned pt_num, CC_show::const_CC_sheet_iterator_t c_sheet, AnimateState& state);

public:
    bool Append(std::shared_ptr<AnimateCommand> cmd, const ContToken* token);
    void RegisterError(AnimateError err, const ContToken* token) const;

    float GetVarValue(int varnum, const ContToken* token) const;
    void SetVarValue(int varnum, float value);

    // helper functions to get information for building a command
    auto GetPointPosition() const { return mState.pt; }
    auto GetCurrentPoint() const { return curr_pt; }
    auto GetBeatsRemaining() const { return mState.beats_rem; }
    AnimatePoint GetStartingPosition() const;
    AnimatePoint GetEndingPosition(const ContToken* token) const;
    AnimatePoint GetReferencePointPosition(unsigned refnum) const;

private:
    const CC_show& mShow;
    const SYMBOL_TYPE contsymbol;
    const unsigned curr_pt;
    const CC_show::const_CC_sheet_iterator_t curr_sheet;

    AnimateState& mState;
};
