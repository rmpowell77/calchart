//
//  e7_transition_solver.hpp
//  CalChart
//
//  Created by Kevin Durand on 10/12/16.
//
//

#pragma once

#include <vector>
#include "cc_coord.h"
#include "cc_sheet.h"



std::tuple<bool, std::vector<CC_coord>, std::vector<CC_coord>, std::vector<std::string>, std::vector<SYMBOL_TYPE>> runSolver(const CC_sheet& sheet1, const CC_sheet& sheet2, unsigned numBeats);