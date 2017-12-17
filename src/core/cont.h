#pragma once
/*
 * cont.h
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

#include "animatecompile.h"
#include <iosfwd>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/unique_ptr.hpp>

// what follows it effectively the Abstract Syntax Tree for CalChart continuities.
//
// Everything derives from a ContToken, which is the basic parsing unit.
// To make the objects more "value" type, there is a clone and is_equal function they
// all implement.
// We use boost::serialize to archive and restore these objects.
// We have both raw pointer constructors and unique_ptr constructors.
// The raw pointer constructors come from the cont parser, and implies that the newly
// formed objects owns the pointers it was given

namespace CalChart {

enum ContDefinedValue {
    CC_N,
    CC_NW,
    CC_W,
    CC_SW,
    CC_S,
    CC_SE,
    CC_E,
    CC_NE,
    CC_HS,
    CC_MM,
    CC_SH,
    CC_JS,
    CC_GV,
    CC_M,
    CC_DM
};

enum class ContType {
    procedure,
    value,
    point,
};

struct DrawableCont {
    ContType type = ContType::procedure;
    std::string description;
    std::vector<DrawableCont> args;
};

class ContToken {
public:
    ContToken();
    virtual ~ContToken() = default;
    virtual std::ostream& Print(std::ostream&) const;

    int line, col;

private:
    friend bool operator==(ContToken const& lhs, ContToken const& rhs);
    virtual bool is_equal(ContToken const& other) const
    {
        return line == other.line && col == other.col;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& line;
        ar& col;
    }
};

inline std::ostream& operator<<(std::ostream& os, const ContToken& c)
{
    return c.Print(os);
}

inline bool operator==(ContToken const& lhs, ContToken const& rhs)
{
    return (typeid(lhs) == typeid(rhs)) && lhs.is_equal(rhs);
}

class ContPoint : public ContToken {
    using super = ContToken;

public:
    ContPoint() {}
    virtual Coord Get(AnimateCompile& anim) const;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const;
    virtual std::unique_ptr<ContPoint> clone() const;

private:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return true;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContStartPoint : public ContPoint {
    using super = ContPoint;

public:
    ContStartPoint() {}
    virtual Coord Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

private:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return true;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContNextPoint : public ContPoint {
    using super = ContPoint;

public:
    ContNextPoint() {}
    virtual Coord Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

private:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return true;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContRefPoint : public ContPoint {
    using super = ContPoint;

public:
    ContRefPoint(unsigned n)
        : refnum(n)
    {
    }
    virtual Coord Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContPoint> clone() const override;

private:
    // default constructor for serialization
    ContRefPoint()
        : refnum(0)
    {
    }

    virtual bool is_equal(ContToken const& other) const override
    {
        return refnum == dynamic_cast<ContRefPoint const&>(other).refnum;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& refnum;
    }

    unsigned refnum;
};

class ContValue : public ContToken {
    using super = ContToken;

public:
    ContValue() {}
    virtual float Get(AnimateCompile& anim) const = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const = 0;
    virtual std::unique_ptr<ContValue> clone() const = 0;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContValueFloat : public ContValue {
    using super = ContValue;

public:
    ContValueFloat(float v)
        : val(v)
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueFloat()
        : val(0)
    {
    }

    virtual bool is_equal(ContToken const& other) const override
    {
        return val == dynamic_cast<ContValueFloat const&>(other).val;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val;
    }

    float val;
};

class ContValueDefined : public ContValue {
    using super = ContValue;

public:
    ContValueDefined(ContDefinedValue v)
        : val(v)
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueDefined()
        : val(CC_N)
    {
    }

    virtual bool is_equal(ContToken const& other) const override
    {
        return val == dynamic_cast<ContValueDefined const&>(other).val;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val;
    }

    ContDefinedValue val;
};

class ContValueAdd : public ContValue {
    using super = ContValue;

public:
    ContValueAdd(ContValue* v1, ContValue* v2)
        : val1(v1)
        , val2(v2)
    {
    }
    ContValueAdd(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
        : val1(std::move(v1))
        , val2(std::move(v2))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueAdd() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueAdd const&>(other);
        return (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val1;
        ar& val2;
    }

    std::unique_ptr<ContValue> val1, val2;
};

class ContValueSub : public ContValue {
    using super = ContValue;

public:
    ContValueSub(ContValue* v1, ContValue* v2)
        : val1(v1)
        , val2(v2)
    {
    }
    ContValueSub(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
        : val1(std::move(v1))
        , val2(std::move(v2))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueSub() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueSub const&>(other);
        return (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val1;
        ar& val2;
    }

    std::unique_ptr<ContValue> val1, val2;
};

class ContValueMult : public ContValue {
    using super = ContValue;

public:
    ContValueMult(ContValue* v1, ContValue* v2)
        : val1(v1)
        , val2(v2)
    {
    }
    ContValueMult(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
        : val1(std::move(v1))
        , val2(std::move(v2))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueMult() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueMult const&>(other);
        return (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val1;
        ar& val2;
    }

    std::unique_ptr<ContValue> val1, val2;
};

class ContValueDiv : public ContValue {
    using super = ContValue;

public:
    ContValueDiv(ContValue* v1, ContValue* v2)
        : val1(v1)
        , val2(v2)
    {
    }
    ContValueDiv(std::unique_ptr<ContValue> v1, std::unique_ptr<ContValue> v2)
        : val1(std::move(v1))
        , val2(std::move(v2))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueDiv() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContValueDiv const&>(other);
        return (*val1 == *der_other.val1) && (*val2 == *der_other.val2);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val1;
        ar& val2;
    }

    std::unique_ptr<ContValue> val1, val2;
};

class ContValueNeg : public ContValue {
    using super = ContValue;

public:
    ContValueNeg(ContValue* v)
        : val(v)
    {
    }
    ContValueNeg(std::unique_ptr<ContValue> v)
        : val(std::move(v))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueNeg() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        return (*val == *dynamic_cast<ContValueNeg const&>(other).val);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& val;
    }

    std::unique_ptr<ContValue> val;
};

class ContValueREM : public ContValue {
    using super = ContValue;

public:
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return true;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContValueVar : public ContValue {
    using super = ContValue;

public:
    ContValueVar(unsigned num)
        : varnum(num)
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    void Set(AnimateCompile& anim, float v);
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContValueVar()
        : varnum(0)
    {
    }

    virtual bool is_equal(ContToken const& other) const override
    {
        return (varnum == dynamic_cast<ContValueVar const&>(other).varnum);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& varnum;
    }

    unsigned varnum;
};

class ContFuncDir : public ContValue {
    using super = ContValue;

public:
    ContFuncDir(ContPoint* p)
        : pnt(p)
    {
    }
    ContFuncDir(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncDir() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        return (*pnt == *dynamic_cast<ContFuncDir const&>(other).pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContFuncDirFrom : public ContValue {
    using super = ContValue;

public:
    ContFuncDirFrom(ContPoint* start, ContPoint* end)
        : pnt_start(start)
        , pnt_end(end)
    {
    }
    ContFuncDirFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end)
        : pnt_start(std::move(start))
        , pnt_end(std::move(end))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncDirFrom() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncDirFrom const&>(other);
        return (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt_start;
        ar& pnt_end;
    }

    std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncDist : public ContValue {
    using super = ContValue;

public:
    ContFuncDist(ContPoint* p)
        : pnt(p)
    {
    }
    ContFuncDist(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncDist() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        return (*pnt == *dynamic_cast<ContFuncDist const&>(other).pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContFuncDistFrom : public ContValue {
    using super = ContValue;

public:
    ContFuncDistFrom(ContPoint* start, ContPoint* end)
        : pnt_start(start)
        , pnt_end(end)
    {
    }
    ContFuncDistFrom(std::unique_ptr<ContPoint> start, std::unique_ptr<ContPoint> end)
        : pnt_start(std::move(start))
        , pnt_end(std::move(end))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncDistFrom() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncDistFrom const&>(other);
        return (*pnt_start == *der_other.pnt_start) && (*pnt_end == *der_other.pnt_end);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt_start;
        ar& pnt_end;
    }

    std::unique_ptr<ContPoint> pnt_start, pnt_end;
};

class ContFuncEither : public ContValue {
    using super = ContValue;

public:
    ContFuncEither(ContValue* d1, ContValue* d2, ContPoint* p)
        : dir1(d1)
        , dir2(d2)
        , pnt(p)
    {
    }
    ContFuncEither(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContPoint> p)
        : dir1(std::move(d1))
        , dir2(std::move(d2))
        , pnt(std::move(p))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncEither() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncEither const&>(other);
        return (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& dir1;
        ar& dir2;
        ar& pnt;
    }

    std::unique_ptr<ContValue> dir1, dir2;
    std::unique_ptr<ContPoint> pnt;
};

class ContFuncOpp : public ContValue {
    using super = ContValue;

public:
    ContFuncOpp(ContValue* d)
        : dir(d)
    {
    }
    ContFuncOpp(std::unique_ptr<ContValue> d)
        : dir(std::move(d))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncOpp() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        return (*dir == *dynamic_cast<ContFuncOpp const&>(other).dir);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& dir;
    }

    std::unique_ptr<ContValue> dir;
};

class ContFuncStep : public ContValue {
    using super = ContValue;

public:
    ContFuncStep(ContValue* beats, ContValue* blocksize, ContPoint* p)
        : numbeats(beats)
        , blksize(blocksize)
        , pnt(p)
    {
    }
    ContFuncStep(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> blocksize, std::unique_ptr<ContPoint> p)
        : numbeats(std::move(beats))
        , blksize(std::move(blocksize))
        , pnt(std::move(p))
    {
    }
    virtual float Get(AnimateCompile& anim) const override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContValue> clone() const override;

private:
    // default constructor for serialization
    ContFuncStep() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContFuncStep const&>(other);
        return (*numbeats == *der_other.numbeats) && (*blksize == *der_other.blksize) && (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& numbeats;
        ar& blksize;
        ar& pnt;
    }

    std::unique_ptr<ContValue> numbeats, blksize;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcedure : public ContToken {
    using super = ContToken;

public:
    ContProcedure() {}
    virtual void Compile(AnimateCompile& anim) = 0;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const = 0;
    virtual std::unique_ptr<ContProcedure> clone() const = 0;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContProcSet : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcSet(ContValueVar* vr, ContValue* v)
        : var(vr)
        , val(v)
    {
    }
    ContProcSet(std::unique_ptr<ContValueVar> vr, std::unique_ptr<ContValue> v)
        : var(std::move(vr))
        , val(std::move(v))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcSet() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcSet const&>(other);
        return (*var == *der_other.var) && (*val == *der_other.val);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& var;
        ar& val;
    }

    std::unique_ptr<ContValueVar> var;
    std::unique_ptr<ContValue> val;
};

class ContProcBlam : public ContProcedure {
    using super = ContProcedure;

public:
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // we use the assumption that we've already checked that the types match before calling.
    virtual bool is_equal(ContToken const& other) const override
    {
        return true;
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
    }
};

class ContProcCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcCM(ContPoint* p1, ContPoint* p2, ContValue* steps, ContValue* d1,
        ContValue* d2, ContValue* beats)
        : pnt1(p1)
        , pnt2(p2)
        , stps(steps)
        , dir1(d1)
        , dir2(d2)
        , numbeats(beats)
    {
    }
    ContProcCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d1,
        std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> beats)
        : pnt1(std::move(p1))
        , pnt2(std::move(p2))
        , stps(std::move(steps))
        , dir1(std::move(d1))
        , dir2(std::move(d2))
        , numbeats(std::move(beats))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcCM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcCM const&>(other);
        return (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*stps == *der_other.stps) && (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*numbeats == *der_other.numbeats);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt1;
        ar& pnt2;
        ar& stps;
        ar& dir1;
        ar& dir2;
        ar& numbeats;
    }

    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> stps, dir1, dir2, numbeats;
};

class ContProcDMCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcDMCM(ContPoint* p1, ContPoint* p2, ContValue* beats)
        : pnt1(p1)
        , pnt2(p2)
        , numbeats(beats)
    {
    }
    ContProcDMCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats)
        : pnt1(std::move(p1))
        , pnt2(std::move(p2))
        , numbeats(std::move(beats))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcDMCM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcDMCM const&>(other);
        return (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt1;
        ar& pnt2;
        ar& numbeats;
    }

    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> numbeats;
};

class ContProcDMHS : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcDMHS(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcDMHS(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcDMHS() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcDMHS const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcEven : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcEven(ContValue* steps, ContPoint* p)
        : stps(steps)
        , pnt(p)
    {
    }
    ContProcEven(std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p)
        : stps(std::move(steps))
        , pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcEven() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcEven const&>(other);
        return (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& stps;
        ar& pnt;
    }

    std::unique_ptr<ContValue> stps;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcEWNS : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcEWNS(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcEWNS(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcEWNS() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcEWNS const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcFountain : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFountain(ContValue* d1, ContValue* d2, ContValue* s1, ContValue* s2,
        ContPoint* p)
        : dir1(d1)
        , dir2(d2)
        , stepsize1(s1)
        , stepsize2(s2)
        , pnt(p)
    {
    }
    ContProcFountain(std::unique_ptr<ContValue> d1, std::unique_ptr<ContValue> d2, std::unique_ptr<ContValue> s1, std::unique_ptr<ContValue> s2,
        std::unique_ptr<ContPoint> p)
        : dir1(std::move(d1))
        , dir2(std::move(d2))
        , stepsize1(std::move(s1))
        , stepsize2(std::move(s2))
        , pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcFountain() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcFountain const&>(other);
        return (*dir1 == *der_other.dir1) && (*dir2 == *der_other.dir2) && (*stepsize1 == *der_other.stepsize1) && (*stepsize2 == *der_other.stepsize2) && (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& dir1;
        ar& dir2;
        ar& stepsize1;
        ar& stepsize2;
        ar& pnt;
    }

    std::unique_ptr<ContValue> dir1, dir2;
    std::unique_ptr<ContValue> stepsize1, stepsize2;
    std::unique_ptr<ContPoint> pnt;
};

class ContProcFM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFM(ContValue* steps, ContValue* d)
        : stps(steps)
        , dir(d)
    {
    }
    ContProcFM(std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d)
        : stps(std::move(steps))
        , dir(std::move(d))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcFM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcFM const&>(other);
        return (*stps == *der_other.stps) && (*dir == *der_other.dir);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& stps;
        ar& dir;
    }

    std::unique_ptr<ContValue> stps, dir;
};

class ContProcFMTO : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcFMTO(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcFMTO(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcFMTO() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcFMTO const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcGrid : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcGrid(ContValue* g)
        : grid(g)
    {
    }
    ContProcGrid(std::unique_ptr<ContValue> g)
        : grid(std::move(g))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcGrid() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcGrid const&>(other);
        return (*grid == *der_other.grid);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& grid;
    }

    std::unique_ptr<ContValue> grid;
};

class ContProcHSCM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcHSCM(ContPoint* p1, ContPoint* p2, ContValue* beats)
        : pnt1(p1)
        , pnt2(p2)
        , numbeats(beats)
    {
    }
    ContProcHSCM(std::unique_ptr<ContPoint> p1, std::unique_ptr<ContPoint> p2, std::unique_ptr<ContValue> beats)
        : pnt1(std::move(p1))
        , pnt2(std::move(p2))
        , numbeats(std::move(beats))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcHSCM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcHSCM const&>(other);
        return (*pnt1 == *der_other.pnt1) && (*pnt2 == *der_other.pnt2) && (*numbeats == *der_other.numbeats);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt1;
        ar& pnt2;
        ar& numbeats;
    }

    std::unique_ptr<ContPoint> pnt1, pnt2;
    std::unique_ptr<ContValue> numbeats;
};

class ContProcHSDM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcHSDM(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcHSDM(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcHSDM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcHSDM const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcMagic : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMagic(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcMagic(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcMagic() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMagic const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcMarch : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMarch(ContValue* stepsize, ContValue* steps, ContValue* d,
        ContValue* face)
        : stpsize(stepsize)
        , stps(steps)
        , dir(d)
        , facedir(face)
    {
    }
    ContProcMarch(std::unique_ptr<ContValue> stepsize, std::unique_ptr<ContValue> steps, std::unique_ptr<ContValue> d,
        std::unique_ptr<ContValue> face)
        : stpsize(std::move(stepsize))
        , stps(std::move(steps))
        , dir(std::move(d))
        , facedir(std::move(face))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcMarch() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMarch const&>(other);
        return (*stpsize == *der_other.stpsize) && (*stps == *der_other.stps) && (*dir == *der_other.dir) && (*facedir == *der_other.facedir);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& stpsize;
        ar& stps;
        ar& dir;
        ar& facedir;
    }

    std::unique_ptr<ContValue> stpsize, stps, dir, facedir;
};

class ContProcMT : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMT(ContValue* beats, ContValue* d)
        : numbeats(beats)
        , dir(d)
    {
    }
    ContProcMT(std::unique_ptr<ContValue> beats, std::unique_ptr<ContValue> d)
        : numbeats(std::move(beats))
        , dir(std::move(d))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcMT() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMT const&>(other);
        return (*numbeats == *der_other.numbeats) && (*dir == *der_other.dir);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& numbeats;
        ar& dir;
    }

    std::unique_ptr<ContValue> numbeats, dir;
};

class ContProcMTRM : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcMTRM(ContValue* d)
        : dir(d)
    {
    }
    ContProcMTRM(std::unique_ptr<ContValue> d)
        : dir(std::move(d))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcMTRM() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcMTRM const&>(other);
        return (*dir == *der_other.dir);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& dir;
    }

    std::unique_ptr<ContValue> dir;
};

class ContProcNSEW : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcNSEW(ContPoint* p)
        : pnt(p)
    {
    }
    ContProcNSEW(std::unique_ptr<ContPoint> p)
        : pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcNSEW() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcNSEW const&>(other);
        return (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& pnt;
    }

    std::unique_ptr<ContPoint> pnt;
};

class ContProcRotate : public ContProcedure {
    using super = ContProcedure;

public:
    ContProcRotate(ContValue* angle, ContValue* steps, ContPoint* p)
        : ang(angle)
        , stps(steps)
        , pnt(p)
    {
    }
    ContProcRotate(std::unique_ptr<ContValue> angle, std::unique_ptr<ContValue> steps, std::unique_ptr<ContPoint> p)
        : ang(std::move(angle))
        , stps(std::move(steps))
        , pnt(std::move(p))
    {
    }
    virtual void Compile(AnimateCompile& anim) override;
    virtual std::ostream& Print(std::ostream&) const override;
    virtual DrawableCont GetDrawableCont() const override;
    virtual std::unique_ptr<ContProcedure> clone() const override;

private:
    // default constructor for serialization
    ContProcRotate() {}

    virtual bool is_equal(ContToken const& other) const override
    {
        auto&& der_other = dynamic_cast<ContProcRotate const&>(other);
        return (*ang == *der_other.ang) && (*stps == *der_other.stps) && (*pnt == *der_other.pnt);
    }

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        // serialize base class information
        ar& boost::serialization::base_object<super>(*this);
        ar& ang;
        ar& stps;
        ar& pnt;
    }

    std::unique_ptr<ContValue> ang, stps;
    std::unique_ptr<ContPoint> pnt;
};
}
