/*
 * cont.cpp
 * Classes for continuity
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

#include "cont.h"
#include "animatecommand.h"
#include "animatecompile.h"
#include "cc_sheet.h"
#include "math_utils.h"
#include "parse.h"

// for serialization we need to pre-register all of the different types that can exist in the continuity AST.
BOOST_CLASS_EXPORT_GUID(CalChart::ContToken, "ContToken")
BOOST_CLASS_EXPORT_GUID(CalChart::ContPointUnset, "ContPointUnset")
BOOST_CLASS_EXPORT_GUID(CalChart::ContPoint, "ContPoint")
BOOST_CLASS_EXPORT_GUID(CalChart::ContStartPoint, "ContStartPoint")
BOOST_CLASS_EXPORT_GUID(CalChart::ContNextPoint, "ContNextPoint")
BOOST_CLASS_EXPORT_GUID(CalChart::ContRefPoint, "ContRefPoint")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValue, "ContValue")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueUnset, "ContValueUnset")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueFloat, "ContValueFloat")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueDefined, "ContValueDefined")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueAdd, "ContValueAdd")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueSub, "ContValueSub")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueMult, "ContValueMult")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueDiv, "ContValueDiv")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueNeg, "ContValueNeg")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueREM, "ContValueREM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContValueVar, "ContValueVar")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncDir, "ContFuncDir")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncDirFrom, "ContFuncDirFrom")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncDist, "ContFuncDist")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncDistFrom, "ContFuncDistFrom")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncEither, "ContFuncEither")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncOpp, "ContFuncOpp")
BOOST_CLASS_EXPORT_GUID(CalChart::ContFuncStep, "ContFuncStep")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcedure, "ContProcedure")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcUnset, "ContProcUnset")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcSet, "ContProcSet")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcBlam, "ContProcBlam")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcCM, "ContProcCM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcDMCM, "ContProcDMCM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcDMHS, "ContProcDMHS")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcEven, "ContProcEven")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcEWNS, "ContProcEWNS")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcFountain, "ContProcFountain")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcFM, "ContProcFM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcFMTO, "ContProcFMTO")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcGrid, "ContProcGrid")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcHSCM, "ContProcHSCM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcHSDM, "ContProcHSDM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcMagic, "ContProcMagic")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcMarch, "ContProcMarch")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcMT, "ContProcMT")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcMTRM, "ContProcMTRM")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcNSEW, "ContProcNSEW")
BOOST_CLASS_EXPORT_GUID(CalChart::ContProcRotate, "ContProcRotate")

namespace CalChart {

// helper for setting the parent pointer
template <typename P, typename T>
void SetParentPtr_helper(P parent, T& t)
{
    if (t) {
        t->SetParentPtr(parent);
    }
}

template <typename P, typename T, typename... Ts>
void SetParentPtr_helper(P parent, T& t, Ts&... ts)
{
    if (t) {
        t->SetParentPtr(parent);
    }
    SetParentPtr_helper(parent, ts...);
}

// helper for some dynamic casting
template <typename Derived, typename Base>
std::unique_ptr<Derived>
dynamic_unique_ptr_cast(std::unique_ptr<Base>&& p)
{
    if (Derived* result = dynamic_cast<Derived*>(p.get())) {
        p.release();
        return std::unique_ptr<Derived>(result);
    }
    return std::unique_ptr<Derived>(nullptr);
}

// helper function that examines each pointer to see if it shuold be replaced, and then replace it.
// making sure to clean things up a the end.
// helper function that examines each pointer to see if it shuold be replaced, and then replace it.
// making sure to clean things up a the end.
template <typename R, typename UP, typename T>
void replace_helper2(R replace, UP& new_value, T& t)
{
    if (t.get() == replace) {
        using Derived = typename T::element_type;
        auto result = dynamic_cast<Derived*>(new_value.get());
        if (!result) {
            throw std::runtime_error("Invalid value in replace");
        }
        auto&& casted = dynamic_unique_ptr_cast<Derived>(std::move(new_value));
        if (!casted) {
            throw std::runtime_error("Invalid value in replace");
        }
        t = std::move(casted);
    }
}

template <typename R, typename UP, typename T, typename... Ts>
void replace_helper2(R replace, UP& new_value, T& t, Ts&... ts)
{
    if (t.get() == replace) {
        using Derived = typename T::element_type;
        auto result = dynamic_cast<Derived*>(new_value.get());
        if (!result) {
            throw std::runtime_error("Invalid value in replace");
        }
        auto&& casted = dynamic_unique_ptr_cast<Derived>(std::move(new_value));
        if (!casted) {
            throw std::runtime_error("Invalid value in replace");
        }
        t = std::move(casted);
    } else {
        replace_helper2(replace, new_value, ts...);
    }
}

template <typename P, typename R, typename UP, typename T, typename... Ts>
void replace_helper(P parent, R replace, UP& new_value, T& t, Ts&... ts)
{
    replace_helper2(replace, new_value, t, ts...);
    SetParentPtr_helper(parent, t, ts...);
}

static const std::string ContDefinedValue_strings[] = {
    "N", "NW", "W", "SW", "S", "SE", "E", "NE",
    "HS", "MM", "SH", "JS", "GV", "M", "DM"
};

int float2int(const ContProcedure* proc, AnimateCompile& anim, float f)
{
    auto v = (int)floor(f + 0.5);
    if (std::abs(f - (float)v) >= COORD_DECIMAL) {
        anim.RegisterError(ANIMERR_NONINT, proc);
    }
    return v;
}

unsigned float2unsigned(const ContProcedure* proc, AnimateCompile& anim,
    float f)
{
    auto v = float2int(proc, anim, f);
    if (v < 0) {
        anim.RegisterError(ANIMERR_NEGINT, proc);
        return 0;
    } else {
        return (unsigned)v;
    }
}

void DoCounterMarch(const ContProcedure& proc, AnimateCompile& anim,
    const ContPoint& pnt1, const ContPoint& pnt2,
    const ContValue& stps, const ContValue& dir1,
    const ContValue& dir2, const ContValue& numbeats)
{
    auto d1 = dir1.Get(anim);
    auto d2 = dir2.Get(anim);
    auto c = sin(Deg2Rad(d1 - d2));
    if (IS_ZERO(c)) {
        anim.RegisterError(ANIMERR_INVALID_CM, &proc);
        return;
    }
    auto ref1 = pnt1.Get(anim);
    auto ref2 = pnt2.Get(anim);
    auto steps1 = stps.Get(anim);
    auto beats = numbeats.Get(anim);

    auto v1 = CreateVector(d1, steps1);

    Coord p[4];
    p[1] = ref1 + v1;
    auto steps2 = (ref2 - p[1]).Magnitude() * sin(Deg2Rad(ref2.Direction(p[1]) - d1)) / c;
    if (IsDiagonalDirection(d2)) {
        steps2 /= static_cast<float>(SQRT2);
    }
    auto v2 = CreateVector(d2, steps2);
    p[2] = p[1] + v2;
    p[3] = ref2 - v1;
    p[0] = p[3] - v2;

    v1 = p[1] - anim.GetPointPosition();
    c = BoundDirectionSigned(v1.Direction() - d1);
    auto leg = 0;
    if ((v1 != 0) && (IS_ZERO(c))) {
        leg = 1;
    } else {
        v1 = p[2] - anim.GetPointPosition();
        c = BoundDirectionSigned(v1.Direction() - d2);
        if ((v1 != 0) && (IS_ZERO(c))) {
            leg = 2;
        } else {
            v1 = p[3] - anim.GetPointPosition();
            c = BoundDirectionSigned(v1.Direction() - d1 - 180.0f);
            if ((v1 != 0) && (IS_ZERO(c))) {
                leg = 3;
            } else {
                v1 = p[0] - anim.GetPointPosition();
                c = BoundDirectionSigned(v1.Direction() - d2 - 180.0f);
                if ((v1 != 0) && (IS_ZERO(c))) {
                    leg = 0;
                } else {
                    // Current point is not in path of countermarch
                    anim.RegisterError(ANIMERR_INVALID_CM, &proc);
                    return;
                }
            }
        }
    }

    while (beats > 0) {
        v1 = p[leg] - anim.GetPointPosition();
        c = v1.DM_Magnitude();
        if (c <= beats) {
            beats -= c;
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(&proc, anim, c), v1),
                    &proc)) {
                return;
            }
        } else {
            switch (leg) {
            case 0:
                v1 = CreateVector(d2 + 180.0f, beats);
                break;
            case 1:
                v1 = CreateVector(d1, beats);
                break;
            case 2:
                v1 = CreateVector(d2, beats);
                break;
            default:
                v1 = CreateVector(d1 + 180.0f, beats);
                break;
            }
            anim.Append(std::make_shared<AnimateCommandMove>(
                            float2unsigned(&proc, anim, beats), v1),
                &proc);
            return;
        }
        leg++;
        if (leg > 3)
            leg = 0;
    }
}

ContToken::ContToken()
    : line(yylloc.first_line)
    , col(yylloc.first_column)
{
}

std::ostream& ContToken::Print(std::ostream& os) const
{
    return os << "[" << line << "," << col << "]: ";
}

void ContToken::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    throw std::runtime_error("Error, replace not implemented on this class");
}

Coord ContPoint::Get(AnimateCompile& anim) const
{
    return anim.GetPointPosition();
}

std::ostream& ContPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Point:";
}

DrawableCont ContPoint::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::point,
        "Point",
        "P",
        {}
    };
}

std::unique_ptr<ContPoint> ContPoint::clone() const
{
    return std::make_unique<ContPoint>();
}

std::ostream& ContPointUnset::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Unset";
}

DrawableCont ContPointUnset::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::unset,
        "unset point",
        "unset point",
        {}
    };
}

std::unique_ptr<ContPoint> ContPointUnset::clone() const
{
    return std::make_unique<ContPointUnset>();
}

Coord ContStartPoint::Get(AnimateCompile& anim) const
{
    return anim.GetStartingPosition();
}

std::ostream& ContStartPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Start Point";
}

DrawableCont ContStartPoint::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::point,
        "Start Point",
        "SP",
        {}
    };
}

std::unique_ptr<ContPoint> ContStartPoint::clone() const
{
    return std::make_unique<ContStartPoint>();
}

Coord ContNextPoint::Get(AnimateCompile& anim) const
{
    return anim.GetEndingPosition(this);
}

std::ostream& ContNextPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Next Point";
}

DrawableCont ContNextPoint::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::point,
        "Next Point",
        "NP",
        {}
    };
}

std::unique_ptr<ContPoint> ContNextPoint::clone() const
{
    return std::make_unique<ContNextPoint>();
}

ContRefPoint::ContRefPoint(unsigned n)
    : refnum(n)
{
}

ContRefPoint::ContRefPoint()
    : refnum(0)
{
}

Coord ContRefPoint::Get(AnimateCompile& anim) const
{
    return anim.GetReferencePointPosition(refnum);
}

std::ostream& ContRefPoint::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Ref Point " << refnum;
}

DrawableCont ContRefPoint::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::point,
        std::string("Ref Point ") + std::to_string(refnum),
        std::string("R") + std::to_string(refnum),
        {}
    };
}

std::unique_ptr<ContPoint> ContRefPoint::clone() const
{
    return std::make_unique<ContRefPoint>(refnum);
}

std::ostream& ContValue::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Value:";
}

std::ostream& ContValueUnset::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Unset";
}

DrawableCont ContValueUnset::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::unset,
        "unset value",
        "unset value",
        {}
    };
}

std::unique_ptr<ContValue> ContValueUnset::clone() const
{
    // we need to make a copy of the var, then dynamically cast to std::unique_ptr<ContValueVar>
    return std::make_unique<ContValueUnset>();
}

ContValueFloat::ContValueFloat(float v)
    : val(v)
{
}

ContValueFloat::ContValueFloat()
    : val(0)
{
}

float ContValueFloat::Get(AnimateCompile&) const { return val; }

std::ostream& ContValueFloat::Print(std::ostream& os) const
{
    super::Print(os);
    return os << val;
}

DrawableCont ContValueFloat::GetDrawableCont() const
{
    // to_string gives a lot of decimal points.  256 on the stack should be ok...?
    if (int(val) == val) {
        return { this, parent_ptr, ContType::value, std::to_string(int(val)), std::to_string(int(val)), {} };
    }
    return { this, parent_ptr, ContType::value, std::to_string(val), std::to_string(val), {} };
}

std::unique_ptr<ContValue> ContValueFloat::clone() const
{
    return std::make_unique<ContValueFloat>(val);
}

ContValueDefined::ContValueDefined(ContDefinedValue v)
    : val(v)
{
}

ContValueDefined::ContValueDefined()
    : val(CC_N)
{
}

float ContValueDefined::Get(AnimateCompile&) const
{
    static const std::map<ContDefinedValue, float> mapping = {
        { CC_NW, 45.0 },
        { CC_W, 90.0 },
        { CC_SW, 135.0 },
        { CC_S, 180.0 },
        { CC_SE, 225.0 },
        { CC_E, 270.0 },
        { CC_NE, 315.0 },
        { CC_HS, 1.0 },
        { CC_MM, 1.0 },
        { CC_SH, 0.5 },
        { CC_JS, 0.5 },
        { CC_GV, 1.0 },
        { CC_M, 4.0f / 3 },
        { CC_DM, static_cast<float>(SQRT2) },
    };
    auto i = mapping.find(val);
    if (i != mapping.end()) {
        return i->second;
    }
    return 0.0;
}

std::ostream& ContValueDefined::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Defined:" << ContDefinedValue_strings[val];
}

DrawableCont ContValueDefined::GetDrawableCont() const
{
    // to_string gives a lot of decimal points.  256 on the stack should be ok...?
    auto type = ContType::value;
    switch (val) {
    case CC_NW:
    case CC_W:
    case CC_SW:
    case CC_S:
    case CC_SE:
    case CC_E:
    case CC_NE:
    case CC_N:
        type = ContType::direction;
        break;
    default:
        type = ContType::steptype;
    }
    return { this, parent_ptr, type, ContDefinedValue_strings[val], ContDefinedValue_strings[val], {} };
}

std::unique_ptr<ContValue> ContValueDefined::clone() const
{
    return std::make_unique<ContValueDefined>(val);
}

ContValueAdd::ContValueAdd(ContValue* v1, ContValue* v2)
    : val1(v1)
    , val2(v2)
{
    SetParentPtr_helper(this, val1, val2);
}

ContValueAdd::ContValueAdd(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
    : val1(std::move(v1))
    , val2(std::move(v2))
{
    SetParentPtr_helper(this, val1, val2);
}

float ContValueAdd::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) + val2->Get(anim));
}

std::ostream& ContValueAdd::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " + " << *val2;
}

DrawableCont ContValueAdd::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "( %@ + %@ )",
        "(%@+%@)",
        { val1->GetDrawableCont(), val2->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContValueAdd::clone() const
{
    return std::make_unique<ContValueAdd>(val1->clone(), val2->clone());
}

void ContValueAdd::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, val1, val2);
}

ContValueSub::ContValueSub(ContValue* v1, ContValue* v2)
    : val1(v1)
    , val2(v2)
{
    SetParentPtr_helper(this, val1, val2);
}

ContValueSub::ContValueSub(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
    : val1(std::move(v1))
    , val2(std::move(v2))
{
    SetParentPtr_helper(this, val1, val2);
}

float ContValueSub::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) - val2->Get(anim));
}

std::ostream& ContValueSub::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " - " << *val2;
}

DrawableCont ContValueSub::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "( %@ - %@ )",
        "(%@-%@)",
        { val1->GetDrawableCont(), val2->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContValueSub::clone() const
{
    return std::make_unique<ContValueSub>(val1->clone(), val2->clone());
}

void ContValueSub::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, val1, val2);
}

ContValueMult::ContValueMult(ContValue* v1, ContValue* v2)
    : val1(v1)
    , val2(v2)
{
    SetParentPtr_helper(this, val1, val2);
}

ContValueMult::ContValueMult(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
    : val1(std::move(v1))
    , val2(std::move(v2))
{
    SetParentPtr_helper(this, val1, val2);
}

float ContValueMult::Get(AnimateCompile& anim) const
{
    return (val1->Get(anim) * val2->Get(anim));
}

std::ostream& ContValueMult::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " * " << *val2;
}

DrawableCont ContValueMult::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "( %@ * %@ )",
        "(%@*%@)",
        { val1->GetDrawableCont(), val2->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContValueMult::clone() const
{
    return std::make_unique<ContValueMult>(val1->clone(), val2->clone());
}

void ContValueMult::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, val1, val2);
}

ContValueDiv::ContValueDiv(ContValue* v1, ContValue* v2)
    : val1(v1)
    , val2(v2)
{
    SetParentPtr_helper(this, val1, val2);
}

ContValueDiv::ContValueDiv(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
    : val1(std::move(v1))
    , val2(std::move(v2))
{
    SetParentPtr_helper(this, val1, val2);
}

float ContValueDiv::Get(AnimateCompile& anim) const
{
    auto f = val2->Get(anim);
    if (IS_ZERO(f)) {
        anim.RegisterError(ANIMERR_DIVISION_ZERO, this);
        return 0.0;
    } else {
        return (val1->Get(anim) / f);
    }
}

std::ostream& ContValueDiv::Print(std::ostream& os) const
{
    super::Print(os);
    return os << *val1 << " / " << *val2;
}

DrawableCont ContValueDiv::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "( %@ / %@ )",
        "(%@/%@)",
        { val1->GetDrawableCont(), val2->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContValueDiv::clone() const
{
    return std::make_unique<ContValueDiv>(val1->clone(), val2->clone());
}

void ContValueDiv::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, val1, val2);
}

ContValueNeg::ContValueNeg(ContValue* v)
    : val(v)
{
    SetParentPtr_helper(this, val);
}

ContValueNeg::ContValueNeg(std::unique_ptr<ContValue> v)
    : val(std::move(v))
{
    SetParentPtr_helper(this, val);
}

float ContValueNeg::Get(AnimateCompile& anim) const { return -val->Get(anim); }

std::ostream& ContValueNeg::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "- " << *val;
}

DrawableCont ContValueNeg::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "-%@",
        "-%@",
        { val->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContValueNeg::clone() const
{
    return std::make_unique<ContValueNeg>(val->clone());
}

void ContValueNeg::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, val);
}

float ContValueREM::Get(AnimateCompile& anim) const
{
    return anim.GetBeatsRemaining();
}

std::ostream& ContValueREM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "REM";
}

DrawableCont ContValueREM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::value,
        "Remaining",
        "REM",
        {}
    };
}

std::unique_ptr<ContValue> ContValueREM::clone() const
{
    return std::make_unique<ContValueREM>();
}

ContValueVar::ContValueVar(unsigned num)
    : varnum(num)
{
}

ContValueVar::ContValueVar()
    : varnum(0)
{
}

float ContValueVar::Get(AnimateCompile& anim) const
{
    return anim.GetVarValue(varnum, this);
}

std::ostream& ContValueVar::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Var " << varnum;
}

DrawableCont ContValueVar::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::value,
        s_contvar_names[varnum],
        s_contvar_names[varnum],
        {}
    };
}

std::unique_ptr<ContValue> ContValueVar::clone() const
{
    return std::make_unique<ContValueVar>(varnum);
}

void ContValueVar::Set(AnimateCompile& anim, float v)
{
    anim.SetVarValue(varnum, v);
}

std::ostream& ContValueVarUnset::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Unset";
}

DrawableCont ContValueVarUnset::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::unset,
        "unset value var",
        "unset value var",
        {}
    };
}

std::unique_ptr<ContValue> ContValueVarUnset::clone() const
{
    return std::make_unique<ContValueVarUnset>();
}

ContFuncDir::ContFuncDir(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContFuncDir::ContFuncDir(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

float ContFuncDir::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim);
    if (c == anim.GetPointPosition()) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
    }
    return anim.GetPointPosition().Direction(c);
}

std::ostream& ContFuncDir::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Direction to " << *pnt;
}

DrawableCont ContFuncDir::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Direction to %@",
        "DIR %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncDir::clone() const
{
    return std::make_unique<ContFuncDir>(pnt->clone());
}

void ContFuncDir::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContFuncDirFrom::ContFuncDirFrom(ContPoint* start, ContPoint* end)
    : pnt_start(start)
    , pnt_end(end)
{
    SetParentPtr_helper(this, pnt_start, pnt_end);
}

ContFuncDirFrom::ContFuncDirFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end)
    : pnt_start(std::move(start))
    , pnt_end(std::move(end))
{
    SetParentPtr_helper(this, pnt_start, pnt_end);
}

float ContFuncDirFrom::Get(AnimateCompile& anim) const
{
    auto start = pnt_start->Get(anim);
    auto end = pnt_end->Get(anim);
    if (start == end) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
    }
    return start.Direction(end);
}

std::ostream& ContFuncDirFrom::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Direction from " << *pnt_start << " to " << *pnt_end;
}

DrawableCont ContFuncDirFrom::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Direction from %@ to %@",
        "DIRFROM %@ to %@",
        { pnt_start->GetDrawableCont(), pnt_end->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncDirFrom::clone() const
{
    return std::make_unique<ContFuncDirFrom>(pnt_start->clone(), pnt_end->clone());
}

void ContFuncDirFrom::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt_start, pnt_end);
}

ContFuncDist::ContFuncDist(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContFuncDist::ContFuncDist(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

float ContFuncDist::Get(AnimateCompile& anim) const
{
    auto vector = pnt->Get(anim) - anim.GetPointPosition();
    return vector.DM_Magnitude();
}

std::ostream& ContFuncDist::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Distance to " << *pnt;
}

DrawableCont ContFuncDist::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Distance to %@",
        "DIST %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncDist::clone() const
{
    return std::make_unique<ContFuncDist>(pnt->clone());
}

void ContFuncDist::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContFuncDistFrom::ContFuncDistFrom(ContPoint* start, ContPoint* end)
    : pnt_start(start)
    , pnt_end(end)
{
    SetParentPtr_helper(this, pnt_start, pnt_end);
}

ContFuncDistFrom::ContFuncDistFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end)
    : pnt_start(std::move(start))
    , pnt_end(std::move(end))
{
    SetParentPtr_helper(this, pnt_start, pnt_end);
}

float ContFuncDistFrom::Get(AnimateCompile& anim) const
{
    auto vector = pnt_end->Get(anim) - pnt_start->Get(anim);
    return vector.Magnitude();
}

std::ostream& ContFuncDistFrom::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Distance from " << *pnt_start << " to " << *pnt_end;
}

DrawableCont ContFuncDistFrom::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Distance from %@ to %@",
        "DISTFROM %@ to %@",
        { pnt_start->GetDrawableCont(), pnt_end->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncDistFrom::clone() const
{
    return std::make_unique<ContFuncDistFrom>(pnt_start->clone(), pnt_end->clone());
}

void ContFuncDistFrom::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt_start, pnt_end);
}

ContFuncEither::ContFuncEither(ContValue* d1, ContValue* d2, ContPoint* p)
    : dir1(d1)
    , dir2(d2)
    , pnt(p)
{
    SetParentPtr_helper(this, dir1, dir2, pnt);
}

ContFuncEither::ContFuncEither(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContPoint> p)
    : dir1(std::move(d1))
    , dir2(std::move(d2))
    , pnt(std::move(p))
{
    SetParentPtr_helper(this, dir1, dir2, pnt);
}

float ContFuncEither::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim);
    if (anim.GetPointPosition() == c) {
        anim.RegisterError(ANIMERR_UNDEFINED, this);
        return dir1->Get(anim);
    }
    auto dir = anim.GetPointPosition().Direction(c);
    auto d1 = dir1->Get(anim) - dir;
    while (d1 > 180)
        d1 -= 360;
    while (d1 < -180)
        d1 += 360;
    auto d2 = dir2->Get(anim) - dir;
    while (d2 > 180)
        d2 -= 360;
    while (d2 < -180)
        d2 += 360;
    return (std::abs(d1) > std::abs(d2)) ? dir2->Get(anim) : dir1->Get(anim);
}

std::ostream& ContFuncEither::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Either direction to " << *dir1 << " or " << *dir2
              << ", depending on whichever is a shorter angle to " << *pnt;
}

DrawableCont ContFuncEither::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Either direction to %@ or %@, depending on whichever is a shorter angle to %@",
        "EITHER %@ or %@, by %@",
        { dir1->GetDrawableCont(), dir2->GetDrawableCont(), pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncEither::clone() const
{
    return std::make_unique<ContFuncEither>(dir1->clone(), dir2->clone(), pnt->clone());
}

void ContFuncEither::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, dir1, dir2, pnt);
}

ContFuncOpp::ContFuncOpp(ContValue* d)
    : dir(d)
{
    SetParentPtr_helper(this, dir);
}

ContFuncOpp::ContFuncOpp(std::unique_ptr<ContValue> d)
    : dir(std::move(d))
{
    SetParentPtr_helper(this, dir);
}

float ContFuncOpp::Get(AnimateCompile& anim) const
{
    return (dir->Get(anim) + 180.0f);
}

std::ostream& ContFuncOpp::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "opposite direction of " << *dir;
}

DrawableCont ContFuncOpp::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "opposite direction of %@",
        "OPP %@",
        { dir->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncOpp::clone() const
{
    return std::make_unique<ContFuncOpp>(dir->clone());
}

void ContFuncOpp::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, dir);
}

ContFuncStep::ContFuncStep(ContValue* beats, ContValue* blocksize, ContPoint* p)
    : numbeats(beats)
    , blksize(blocksize)
    , pnt(p)
{
    SetParentPtr_helper(this, numbeats, blksize, pnt);
}

ContFuncStep::ContFuncStep(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> blocksize, std::unique_ptr<ContPoint> p)
    : numbeats(std::move(beats))
    , blksize(std::move(blocksize))
    , pnt(std::move(p))
{
    SetParentPtr_helper(this, numbeats, blksize, pnt);
}

float ContFuncStep::Get(AnimateCompile& anim) const
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    return (c.DM_Magnitude() * numbeats->Get(anim) / blksize->Get(anim));
}

std::ostream& ContFuncStep::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Step drill at " << *numbeats << " beats for a block size of "
              << *blksize << " from point " << *pnt;
}

DrawableCont ContFuncStep::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::function,
        "Step drill at %@ beats for a block size of %@ from point %@",
        "STEP %@ Beats, %@ size, from %@",
        { numbeats->GetDrawableCont(), blksize->GetDrawableCont(), pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContValue> ContFuncStep::clone() const
{
    return std::make_unique<ContFuncStep>(numbeats->clone(), blksize->clone(), pnt->clone());
}

void ContFuncStep::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, blksize, pnt);
}

std::ostream& ContProcedure::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Procedure: ";
}

std::ostream& ContProcUnset::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Unset";
}

DrawableCont ContProcUnset::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::unset,
        "unset continuity",
        "unset continuity",
        {}
    };
}

std::unique_ptr<ContProcedure> ContProcUnset::clone() const
{
    // we need to make a copy of the var, then dynamically cast to std::unique_ptr<ContValueVar>
    return std::make_unique<ContProcUnset>();
}

ContProcSet::ContProcSet(ContValueVar* vr, ContValue* v)
    : var(vr)
    , val(v)
{
    SetParentPtr_helper(this, var, val);
}

ContProcSet::ContProcSet(std::unique_ptr<ContValueVar> vr, std::unique_ptr<ContValue> v)
    : var(std::move(vr))
    , val(std::move(v))
{
    SetParentPtr_helper(this, var, val);
}

void ContProcSet::Compile(AnimateCompile& anim)
{
    var->Set(anim, val->Get(anim));
}

std::ostream& ContProcSet::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Setting variable " << *var << " to " << *val;
}

DrawableCont ContProcSet::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "variable %@ = %@",
        "%@ = %@",
        { var->GetDrawableCont(), val->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcSet::clone() const
{
    // we need to make a copy of the var, then dynamically cast to std::unique_ptr<ContValueVar>
    auto var_clone = var->clone();
    if (ContValueVar* cast = dynamic_cast<ContValueVar*>(var_clone.get())) {
        std::unique_ptr<ContValueVar> t(cast);
        var_clone.release();
        return std::make_unique<ContProcSet>(std::move(t), val->clone());
    }
    throw std::runtime_error("ContProcSet var was not of type ContValueVar");
}

//class ContProcSet::ReplaceError_NotAVar : public std::runtime_error
//{
//};
//
void ContProcSet::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    if (var.get() == which) {
        // ProcSet is different because we Must have a ValueVar as when replacing
        auto result = dynamic_cast<ContValueVar*>(v.get());
        if (!result) {
            throw ReplaceError_NotAVar{};
        }
        var = dynamic_unique_ptr_cast<ContValueVar>(std::move(v));
    }
    if (val.get() == which) {
        auto result = dynamic_cast<ContValue*>(v.get());
        if (!result) {
            throw std::runtime_error("Invalid value in replace");
        }
        val = dynamic_unique_ptr_cast<ContValue>(std::move(v));
    }
    SetParentPtr_helper(this, var, val);
}

void ContProcBlam::Compile(AnimateCompile& anim)
{
    ContNextPoint np;
    auto c = np.Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_shared<AnimateCommandMove>(anim.GetBeatsRemaining(), c),
        this);
}

std::ostream& ContProcBlam::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "BLAM";
}

DrawableCont ContProcBlam::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "BLAM",
        "BLAM",
        {}
    };
}

std::unique_ptr<ContProcedure> ContProcBlam::clone() const
{
    return std::make_unique<ContProcBlam>();
}

ContProcCM::ContProcCM(ContPoint* p1, ContPoint* p2, ContValue* steps, ContValue* d1,
    ContValue* d2, ContValue* beats)
    : pnt1(p1)
    , pnt2(p2)
    , stps(steps)
    , dir1(d1)
    , dir2(d2)
    , numbeats(beats)
{
    SetParentPtr_helper(this, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

ContProcCM::ContProcCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d1,
    std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> beats)
    : pnt1(std::move(p1))
    , pnt2(std::move(p2))
    , stps(std::move(steps))
    , dir1(std::move(d1))
    , dir2(std::move(d2))
    , numbeats(std::move(beats))
{
    SetParentPtr_helper(this, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

void ContProcCM::Compile(AnimateCompile& anim)
{
    DoCounterMarch(*this, anim, *pnt1, *pnt2, *stps, *dir1, *dir2, *numbeats);
}

std::ostream& ContProcCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "CounterMarch starting at " << *pnt1 << " passing through "
              << *pnt2 << " stepping " << *stps << " off points, first moving "
              << *dir1 << " then " << *dir2 << " for number beats " << *numbeats;
}

DrawableCont ContProcCM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "CounterMarch starting at %@ passing through %@ stepping %@ off points, first moving %@ then %@ for number beats %@",
        "COUNTERMARCH %@ %@ %@, first %@ then %@ for beats %@",
        { pnt1->GetDrawableCont(), pnt2->GetDrawableCont(), stps->GetDrawableCont(), dir1->GetDrawableCont(), dir2->GetDrawableCont(), numbeats->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcCM::clone() const
{
    return std::make_unique<ContProcCM>(pnt1->clone(), pnt2->clone(), stps->clone(), dir1->clone(), dir2->clone(), numbeats->clone());
}

void ContProcCM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt1, pnt2, stps, dir1, dir2, numbeats);
}

ContProcDMCM::ContProcDMCM(ContPoint* p1, ContPoint* p2, ContValue* beats)
    : pnt1(p1)
    , pnt2(p2)
    , numbeats(beats)
{
    SetParentPtr_helper(this, pnt1, pnt2, numbeats);
}

ContProcDMCM::ContProcDMCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats)
    : pnt1(std::move(p1))
    , pnt2(std::move(p2))
    , numbeats(std::move(beats))
{
    SetParentPtr_helper(this, pnt1, pnt2, numbeats);
}

void ContProcDMCM::Compile(AnimateCompile& anim)
{
    ContValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    auto c = r2.x - r1.x;
    if (c == (r2.y - r1.y + Int2CoordUnits(2))) {
        if (c >= 0) {
            ContValueDefined dir1(CC_SW);
            ContValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y - Int2CoordUnits(2))) {
        if (c >= 0) {
            ContValueDefined dir1(CC_SE);
            ContValueDefined dir2(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r1.y - r2.y + Int2CoordUnits(2))) {
        if (c <= 0) {
            ContValueDefined dir1(CC_NW);
            ContValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    } else if (c == (r2.y - r1.y - Int2CoordUnits(2))) {
        if (c <= 0) {
            ContValueDefined dir1(CC_NE);
            ContValueDefined dir2(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dir1, dir2, *numbeats);
            return;
        }
    }
    anim.RegisterError(ANIMERR_INVALID_CM, this);
}

std::ostream& ContProcDMCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Diagonal march CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

DrawableCont ContProcDMCM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Diagonal march CounterMarch starting at %@ passing through %@ for number beats %@",
        "DMCM %@ %@ for beats %@",
        { pnt1->GetDrawableCont(), pnt2->GetDrawableCont(), numbeats->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcDMCM::clone() const
{
    return std::make_unique<ContProcDMCM>(pnt1->clone(), pnt2->clone(), numbeats->clone());
}

void ContProcDMCM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt1, pnt2, numbeats);
}

ContProcDMHS::ContProcDMHS(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcDMHS::ContProcDMHS(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcDMHS::Compile(AnimateCompile& anim)
{
    short b_hs;

    Coord c_hs, c_dm;
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (std::abs(c.x) > std::abs(c.y)) {
        // adjust sign
        c_hs.x = ((c.x < 0) != (c.y < 0)) ? c.x + c.y : c.x - c.y;
        c_hs.y = 0;
        // adjust sign
        c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
        c_dm.y = c.y;
        b_hs = CoordUnits2Int(c_hs.x);
    } else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b_hs = CoordUnits2Int(c_hs.y);
    }
    if (c_dm != 0) {
        auto b = CoordUnits2Int(c_dm.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm),
                this)) {
            return;
        }
    }
    if (c_hs != 0) {
        anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b_hs), c_hs),
            this);
    }
}

std::ostream& ContProcDMHS::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Diagonal march then HighStep to " << *pnt;
}

DrawableCont ContProcDMHS::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Diagonal march then HighStep to %@",
        "DMHS %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcDMHS::clone() const
{
    return std::make_unique<ContProcDMHS>(pnt->clone());
}

void ContProcDMHS::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContProcEven::ContProcEven(ContValue* steps, ContPoint* p)
    : stps(steps)
    , pnt(p)
{
    SetParentPtr_helper(this, stps, pnt);
}

ContProcEven::ContProcEven(std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p)
    : stps(std::move(steps))
    , pnt(std::move(p))
{
    SetParentPtr_helper(this, stps, pnt);
}

void ContProcEven::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    auto steps = float2int(this, anim, stps->Get(anim));
    if (steps < 0) {
        anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-steps, c,
                        -c.Direction()),
            this);
    } else {
        anim.Append(std::make_shared<AnimateCommandMove>((unsigned)steps, c), this);
    }
}

std::ostream& ContProcEven::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Even march of step size " << *stps << " to " << *pnt;
}

DrawableCont ContProcEven::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Even march %@ to %@",
        "EVEN %@ %@",
        { stps->GetDrawableCont(), pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcEven::clone() const
{
    return std::make_unique<ContProcEven>(stps->clone(), pnt->clone());
}

void ContProcEven::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, stps, pnt);
}

ContProcEWNS::ContProcEWNS(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcEWNS::ContProcEWNS(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcEWNS::Compile(AnimateCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ContProcEWNS::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "March EastWest/NorthSouth to " << *pnt;
}

DrawableCont ContProcEWNS::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "EastWest/NorthSouth to %@",
        "EW/NS to %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcEWNS::clone() const
{
    return std::make_unique<ContProcEWNS>(pnt->clone());
}

void ContProcEWNS::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContProcFountain::ContProcFountain(ContValue* d1, ContValue* d2, ContValue* s1, ContValue* s2,
    ContPoint* p)
    : dir1(d1)
    , dir2(d2)
    , stepsize1(s1)
    , stepsize2(s2)
    , pnt(p)
{
    SetParentPtr_helper(this, dir1, dir2, stepsize1, stepsize2, pnt);
}

ContProcFountain::ContProcFountain(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> s1, std::unique_ptr<ContValue> s2,
    std::unique_ptr<ContPoint> p)
    : dir1(std::move(d1))
    , dir2(std::move(d2))
    , stepsize1(std::move(s1))
    , stepsize2(std::move(s2))
    , pnt(std::move(p))
{
    SetParentPtr_helper(this, dir1, dir2, stepsize1, stepsize2, pnt);
}

void ContProcFountain::Compile(AnimateCompile& anim)
{
    float a, b, c, d;

    auto f1 = dir1->Get(anim);
    if (stepsize1) {
        auto f2 = stepsize1->Get(anim);
        a = f2 * cos(Deg2Rad(f1));
        c = f2 * -sin(Deg2Rad(f1));
    } else {
        std::tie(a, c) = CreateUnitVector(f1);
    }
    f1 = dir2->Get(anim);
    if (stepsize2) {
        auto f2 = stepsize2->Get(anim);
        b = f2 * cos(Deg2Rad(f1));
        d = f2 * -sin(Deg2Rad(f1));
    } else {
        std::tie(b, d) = CreateUnitVector(f1);
    }
    auto v = pnt->Get(anim) - anim.GetPointPosition();
    auto e = CoordUnits2Float(v.x);
    auto f = CoordUnits2Float(v.y);
    f1 = a * d - b * c;
    if (IS_ZERO(f1)) {
        if (IS_ZERO(a - b) && IS_ZERO(c - d) && IS_ZERO(e * c - a * f)) {
            // Special case: directions are same
            if (IS_ZERO(c)) {
                f1 = f / c;
            } else {
                f1 = e / a;
            }
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f1), v),
                    this)) {
                return;
            }
        } else {
            anim.RegisterError(ANIMERR_INVALID_FNTN, this);
            return;
        }
    } else {
        auto f2 = (d * e - b * f) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * a);
            v.y = Float2CoordUnits(f2 * c);
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
        f2 = (a * f - c * e) / f1;
        if (!IS_ZERO(f2)) {
            v.x = Float2CoordUnits(f2 * b);
            v.y = Float2CoordUnits(f2 * d);
            if (!anim.Append(std::make_shared<AnimateCommandMove>(
                                 float2unsigned(this, anim, f2), v),
                    this)) {
                return;
            }
        }
    }
}

std::ostream& ContProcFountain::Print(std::ostream& os) const
{
    super::Print(os);
    os << "Fountain step, first going " << *dir1 << " then " << *dir2;
    if (stepsize1)
        os << ", first at " << *stepsize1;
    if (stepsize2)
        os << ", then at " << *stepsize2;
    return os << "ending at " << *pnt;
}

DrawableCont ContProcFountain::GetDrawableCont() const
{
    if (stepsize1 && stepsize2) {
        return {
            this, parent_ptr,
            ContType::procedure,
            "Fountain step, first going %@ then %@, first at %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, then %@, ending %@",
            { dir1->GetDrawableCont(), dir2->GetDrawableCont(), stepsize1->GetDrawableCont(), stepsize2->GetDrawableCont(), pnt->GetDrawableCont() }
        };
    }
    if (stepsize1) {
        return {
            this, parent_ptr,
            ContType::procedure,
            "Fountain step, first going %@ then %@, first at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@ ending %@",
            { dir1->GetDrawableCont(), dir2->GetDrawableCont(), stepsize1->GetDrawableCont(), pnt->GetDrawableCont() }
        };
    }
    if (stepsize2) {
        return {
            this, parent_ptr,
            ContType::procedure,
            "Fountain step, first going %@ then %@, then at %@, ending at %@",
            "FOUNTAIN %@ -> %@, Step %@, ending %@",
            { dir1->GetDrawableCont(), dir2->GetDrawableCont(), stepsize2->GetDrawableCont(), pnt->GetDrawableCont() }
        };
    }
    return {
        this, parent_ptr,
        ContType::procedure,
        "Fountain step, first going %@ then %@, ending at %@",
        "FOUNTAIN %@ -> %@, ending %@",
        { dir1->GetDrawableCont(), dir2->GetDrawableCont(), pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcFountain::clone() const
{
    return std::make_unique<ContProcFountain>(dir1->clone(), dir2->clone(), stepsize1 ? stepsize1->clone() : nullptr, stepsize2 ? stepsize2->clone() : nullptr, pnt->clone());
}

void ContProcFountain::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, dir1, dir2, stepsize1, stepsize2, pnt);
}

ContProcFM::ContProcFM(ContValue* steps, ContValue* d)
    : stps(steps)
    , dir(d)
{
    SetParentPtr_helper(this, stps, dir);
}

ContProcFM::ContProcFM(std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d)
    : stps(std::move(steps))
    , dir(std::move(d))
{
    SetParentPtr_helper(this, stps, dir);
}

void ContProcFM::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto c = CreateVector(dir->Get(anim), stps->Get(anim));
        if (c != 0) {
            if (b < 0) {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c,
                                -c.Direction()),
                    this);
            } else {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ContProcFM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Forward march for steps " << *stps << " in direction " << *dir;
}

DrawableCont ContProcFM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Forward march %@ %@",
        "FM %@ %@",
        { stps->GetDrawableCont(), dir->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcFM::clone() const
{
    return std::make_unique<ContProcFM>(stps->clone(), dir->clone());
}

void ContProcFM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, stps, dir);
}

ContProcFMTO::ContProcFMTO(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcFMTO::ContProcFMTO(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcFMTO::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (c != 0) {
        anim.Append(
            std::make_shared<AnimateCommandMove>((unsigned)c.DM_Magnitude(), c),
            this);
    }
}

std::ostream& ContProcFMTO::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Forward march to " << *pnt;
}

DrawableCont ContProcFMTO::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Forward march to %@",
        "FMTO %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcFMTO::clone() const
{
    return std::make_unique<ContProcFMTO>(pnt->clone());
}

void ContProcFMTO::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

static inline Coord::units roundcoord(Coord::units a, Coord::units mod)
{
    mod = std::abs(mod);
    if (mod > 0) {
        if (a < 0) {
            a = ((a - (mod / 2)) / mod) * mod;
        } else {
            a = ((a + (mod / 2)) / mod) * mod;
        }
    }
    return a;
}

ContProcGrid::ContProcGrid(ContValue* g)
    : grid(g)
{
    SetParentPtr_helper(this, grid);
}

ContProcGrid::ContProcGrid(std::unique_ptr<ContValue> g)
    : grid(std::move(g))
{
    SetParentPtr_helper(this, grid);
}

void ContProcGrid::Compile(AnimateCompile& anim)
{
    auto gridc = Float2CoordUnits(grid->Get(anim));

    Coord c;
    c.x = roundcoord(anim.GetPointPosition().x, gridc);
    // Adjust so 4 step grid will be on visible grid
    c.y = roundcoord(anim.GetPointPosition().y - Int2CoordUnits(2), gridc) + Int2CoordUnits(2);

    c -= anim.GetPointPosition();
    if (c != 0) {
        anim.Append(std::make_shared<AnimateCommandMove>(0, c), this);
    }
}

std::ostream& ContProcGrid::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Move on Grid of " << *grid << " spacing";
}

DrawableCont ContProcGrid::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Move on Grid of %@ spacing",
        "GRID %@",
        { grid->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcGrid::clone() const
{
    return std::make_unique<ContProcGrid>(grid->clone());
}

void ContProcGrid::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, grid);
}

ContProcHSCM::ContProcHSCM(ContPoint* p1, ContPoint* p2, ContValue* beats)
    : pnt1(p1)
    , pnt2(p2)
    , numbeats(beats)
{
    SetParentPtr_helper(this, pnt1, pnt2, numbeats);
}

ContProcHSCM::ContProcHSCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats)
    : pnt1(std::move(p1))
    , pnt2(std::move(p2))
    , numbeats(std::move(beats))
{
    SetParentPtr_helper(this, pnt1, pnt2, numbeats);
}

void ContProcHSCM::Compile(AnimateCompile& anim)
{
    ContValueFloat steps(1.0);

    auto r1 = pnt1->Get(anim);
    auto r2 = pnt2->Get(anim);
    if ((r1.y - r2.y) == Int2CoordUnits(2)) {
        if (r2.x >= r1.x) {
            ContValueDefined dirs(CC_S);
            ContValueDefined dirw(CC_W);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirs, dirw, *numbeats);
            return;
        }
    } else if ((r1.y - r2.y) == -Int2CoordUnits(2)) {
        if (r1.x >= r2.x) {
            ContValueDefined dirn(CC_N);
            ContValueDefined dire(CC_E);
            DoCounterMarch(*this, anim, *pnt1, *pnt2, steps, dirn, dire, *numbeats);
            return;
        }
    }
    anim.RegisterError(ANIMERR_INVALID_CM, this);
}

std::ostream& ContProcHSCM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "High Step CounterMarch starting at " << *pnt1
              << " passing through " << *pnt2 << " for number beats" << *numbeats;
}

DrawableCont ContProcHSCM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "High Step CounterMarch starting at %@ passing through %@ for number beats %@",
        "HSCM %@ -> %@ for beats %@",
        { pnt1->GetDrawableCont(), pnt2->GetDrawableCont(), numbeats->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcHSCM::clone() const
{
    return std::make_unique<ContProcHSCM>(pnt1->clone(), pnt2->clone(), numbeats->clone());
}

void ContProcHSCM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt1, pnt2, numbeats);
}

ContProcHSDM::ContProcHSDM(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcHSDM::ContProcHSDM(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcHSDM::Compile(AnimateCompile& anim)
{
    Coord c_hs, c_dm;
    short b;

    auto c = pnt->Get(anim) - anim.GetPointPosition();
    if (std::abs(c.x) > std::abs(c.y)) {
        // adjust sign
        c_hs.x = ((c.x < 0) != (c.y < 0)) ? c.x + c.y : c.x - c.y;
        c_hs.y = 0;
        // adjust sign
        c_dm.x = ((c.x < 0) != (c.y < 0)) ? -c.y : c.y;
        c_dm.y = c.y;
        b = CoordUnits2Int(c_hs.x);
    } else {
        c_hs.x = 0;
        // adjust sign
        c_hs.y = ((c.x < 0) != (c.y < 0)) ? c.y + c.x : c.y - c.x;
        c_dm.x = c.x;
        // adjust sign
        c_dm.y = ((c.x < 0) != (c.y < 0)) ? -c.x : c.x;
        b = CoordUnits2Int(c_hs.y);
    }
    if (c_hs != 0) {
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_hs),
                this)) {
            return;
        }
    }
    if (c_dm != 0) {
        b = CoordUnits2Int(c_dm.x);
        anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c_dm), this);
    }
}

std::ostream& ContProcHSDM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "HighStep then Diagonal march to " << *pnt;
}

DrawableCont ContProcHSDM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "HighStep then Diagonal march to %@",
        "HSDM %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcHSDM::clone() const
{
    return std::make_unique<ContProcHSDM>(pnt->clone());
}

void ContProcHSDM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContProcMagic::ContProcMagic(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcMagic::ContProcMagic(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcMagic::Compile(AnimateCompile& anim)
{
    auto c = pnt->Get(anim) - anim.GetPointPosition();
    anim.Append(std::make_shared<AnimateCommandMove>(0, c), this);
}

std::ostream& ContProcMagic::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Magic step to " << *pnt;
}

DrawableCont ContProcMagic::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Magic step to %@",
        "MAGIC %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcMagic::clone() const
{
    return std::make_unique<ContProcMagic>(pnt->clone());
}

void ContProcMagic::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContProcMarch::ContProcMarch(ContValue* stepsize, ContValue* steps, ContValue* d,
    ContValue* face)
    : stpsize(stepsize)
    , stps(steps)
    , dir(d)
    , facedir(face)
{
    SetParentPtr_helper(this, stpsize, stps, dir, facedir);
}

ContProcMarch::ContProcMarch(std::unique_ptr<ContValue> stepsize, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d,
    std::unique_ptr<ContValue> face)
    : stpsize(std::move(stepsize))
    , stps(std::move(steps))
    , dir(std::move(d))
    , facedir(std::move(face))
{
    SetParentPtr_helper(this, stpsize, stps, dir, facedir);
}

void ContProcMarch::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, stps->Get(anim));
    if (b != 0) {
        auto rads = Deg2Rad(dir->Get(anim));
        auto mag = stpsize->Get(anim) * stps->Get(anim);
        Coord c{ Float2CoordUnits(cos(rads) * mag),
            static_cast<Coord::units>(-Float2CoordUnits(sin(rads) * mag)) };
        if (c != 0) {
            if (facedir)
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)std::abs(b),
                                c, facedir->Get(anim)),
                    this);
            else if (b < 0) {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)-b, c,
                                -c.Direction()),
                    this);
            } else {
                anim.Append(std::make_shared<AnimateCommandMove>((unsigned)b, c), this);
            }
        }
    }
}

std::ostream& ContProcMarch::Print(std::ostream& os) const
{
    super::Print(os);
    os << "March step size" << *stpsize << " for steps " << *stps
       << " in direction " << *dir;
    if (facedir)
        os << " facing " << *facedir;
    return os;
}

DrawableCont ContProcMarch::GetDrawableCont() const
{
    if (facedir) {
        return {
            this, parent_ptr,
            ContType::procedure,
            "March step size %@ for %@ in direction %@ facing %@",
            "MARCH %@ for %@ DIR %@ FACING %@",
            { stpsize->GetDrawableCont(), stps->GetDrawableCont(), dir->GetDrawableCont(), facedir->GetDrawableCont() }
        };
    }
    return {
        this, parent_ptr,
        ContType::procedure,
        "March step size %@ for steps %@ in direction %@",
        "MARCH %@ for %@ DIR %@",
        { stpsize->GetDrawableCont(), stps->GetDrawableCont(), dir->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcMarch::clone() const
{
    return std::make_unique<ContProcMarch>(stpsize->clone(), stps->clone(), dir->clone(), (facedir) ? facedir->clone() : nullptr);
}

void ContProcMarch::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, stpsize, stps, dir, facedir);
}

ContProcMT::ContProcMT(ContValue* beats, ContValue* d)
    : numbeats(beats)
    , dir(d)
{
    SetParentPtr_helper(this, numbeats, dir);
}

ContProcMT::ContProcMT(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> d)
    : numbeats(std::move(beats))
    , dir(std::move(d))
{
    SetParentPtr_helper(this, numbeats, dir);
}

void ContProcMT::Compile(AnimateCompile& anim)
{
    auto b = float2int(this, anim, numbeats->Get(anim));
    if (b != 0) {
        anim.Append(std::make_shared<AnimateCommandMT>((unsigned)std::abs(b),
                        dir->Get(anim)),
            this);
    }
}

std::ostream& ContProcMT::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "MarkTime for " << *numbeats << " facing " << *dir;
}

DrawableCont ContProcMT::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "MarkTime %@ %@",
        "MT %@ %@",
        { numbeats->GetDrawableCont(), dir->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcMT::clone() const
{
    return std::make_unique<ContProcMT>(numbeats->clone(), dir->clone());
}

void ContProcMT::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, numbeats, dir);
}

ContProcMTRM::ContProcMTRM(ContValue* d)
    : dir(d)
{
    SetParentPtr_helper(this, dir);
}

ContProcMTRM::ContProcMTRM(std::unique_ptr<ContValue> d)
    : dir(std::move(d))
{
    SetParentPtr_helper(this, dir);
}

void ContProcMTRM::Compile(AnimateCompile& anim)
{
    anim.Append(std::make_shared<AnimateCommandMT>(anim.GetBeatsRemaining(),
                    dir->Get(anim)),
        this);
}

std::ostream& ContProcMTRM::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "MarkTime for Remaining Beats facing " << *dir;
}

DrawableCont ContProcMTRM::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "MarkTime for Remaining %@",
        "MTRM %@",
        { dir->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcMTRM::clone() const
{
    return std::make_unique<ContProcMTRM>(dir->clone());
}

void ContProcMTRM::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, dir);
}

ContProcNSEW::ContProcNSEW(ContPoint* p)
    : pnt(p)
{
    SetParentPtr_helper(this, pnt);
}

ContProcNSEW::ContProcNSEW(std::unique_ptr<ContPoint> p)
    : pnt(std::move(p))
{
    SetParentPtr_helper(this, pnt);
}

void ContProcNSEW::Compile(AnimateCompile& anim)
{
    auto c1 = pnt->Get(anim) - anim.GetPointPosition();
    if (c1.x != 0) {
        Coord c2{ c1.x, 0 };
        auto b = CoordUnits2Int(c2.x);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
    if (c1.y != 0) {
        Coord c2{ 0, c1.y };
        auto b = CoordUnits2Int(c2.y);
        if (!anim.Append(std::make_shared<AnimateCommandMove>(std::abs(b), c2),
                this)) {
            return;
        }
    }
}

std::ostream& ContProcNSEW::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "March NorthSouth/EastWest to " << *pnt;
}

DrawableCont ContProcNSEW::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "NorthSouth/EastWest to %@",
        "NSEW %@",
        { pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcNSEW::clone() const
{
    return std::make_unique<ContProcNSEW>(pnt->clone());
}

void ContProcNSEW::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, pnt);
}

ContProcRotate::ContProcRotate(ContValue* angle, ContValue* steps, ContPoint* p)
    : ang(angle)
    , stps(steps)
    , pnt(p)
{
    SetParentPtr_helper(this, ang, stps, pnt);
}

ContProcRotate::ContProcRotate(std::unique_ptr<ContValue> angle, std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p)
    : ang(std::move(angle))
    , stps(std::move(steps))
    , pnt(std::move(p))
{
    SetParentPtr_helper(this, ang, stps, pnt);
}

void ContProcRotate::Compile(AnimateCompile& anim)
{
    // Most of the work is converting to polar coordinates
    auto c = pnt->Get(anim);
    auto rad = anim.GetPointPosition() - c;
    float start_ang;
    if (c == anim.GetPointPosition())
        start_ang = anim.GetVarValue(CONTVAR_DOH, this);
    else
        start_ang = c.Direction(anim.GetPointPosition());
    int b = float2int(this, anim, stps->Get(anim));
    float angle = ang->Get(anim);
    bool backwards = false;
    if (b < 0) {
        backwards = true;
    }
    anim.Append(std::make_shared<AnimateCommandRotate>(
                    (unsigned)std::abs(b), c,
                    // Don't use Magnitude() because
                    // we want Coord numbers
                    sqrt(static_cast<float>(rad.x * rad.x + rad.y * rad.y)),
                    start_ang, start_ang + angle, backwards),
        this);
}

std::ostream& ContProcRotate::Print(std::ostream& os) const
{
    super::Print(os);
    return os << "Rotate at angle " << *ang << " for " << *stps
              << " around pivot point " << *pnt;
}

DrawableCont ContProcRotate::GetDrawableCont() const
{
    return {
        this, parent_ptr,
        ContType::procedure,
        "Rotate at angle %@ for steps %@ around pivot point %@",
        "ROTATE %@ for %@ around %@",
        { ang->GetDrawableCont(), stps->GetDrawableCont(), pnt->GetDrawableCont() }
    };
}

std::unique_ptr<ContProcedure> ContProcRotate::clone() const
{
    return std::make_unique<ContProcRotate>(ang->clone(), stps->clone(), pnt->clone());
}

void ContProcRotate::replace(ContToken const* which, std::unique_ptr<ContToken> v)
{
    replace_helper(this, which, v, ang, stps, pnt);
}
}
