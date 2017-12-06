/*
 * cc_sheet.cpp
 * Defintion for calchart sheet class
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

#include "cc_sheet.h"

#include "cc_show.h"
#include "cc_fileformat.h"
#include "animatecommand.h"
#include "viewer_translate.h"

#include <sstream>
#include <iostream>
#include <map>
#include <cctype>
#include <algorithm>
#include <functional>

const std::string contnames[MAX_NUM_SYMBOLS] = {
    "Plain", "Sol", "Bksl", "Sl", "X", "Solbksl", "Solsl", "Solx"
};

CC_sheet::CC_sheet(size_t numPoints)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mBeats(1)
    , mPoints(numPoints)
{
}

CC_sheet::CC_sheet(size_t numPoints, const std::string& newname)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mBeats(1)
    , mPoints(numPoints)
    , mName(newname)
{
}

// -=-=-=-=-=- LEGACY CODE -=-=-=-=-=-
// Recommend that you don't touch this unless you know what you are doing.
bool are_equal_helper(const std::string& a, const std::string& b)
{
    auto p = std::mismatch(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
        return std::tolower(c1) == std::tolower(c2);
    });
    return (p.first == a.end() && p.second == b.end());
}

bool are_equal(const std::string& a, const std::string& b)
{
    return a.size() <= b.size() ? are_equal_helper(a, b) : are_equal_helper(b, a);
}

SYMBOL_TYPE GetSymbolForName(const std::string& name)
{
    for (auto i = contnames;
         i != (contnames + sizeof(contnames) / sizeof(contnames[0])); ++i) {

        if (are_equal(name, *i)) {
            return static_cast<SYMBOL_TYPE>(std::distance(contnames, i));
        }
    }
    // what do we do here?  give larger one for now...
    // This should probably throw
    return MAX_NUM_SYMBOLS;
}

std::string GetNameForSymbol(SYMBOL_TYPE which)
{
    if (which > MAX_NUM_SYMBOLS) {
        return "";
    }
    return contnames[which];
}

static void
CheckInconsistancy(SYMBOL_TYPE symbol, uint8_t cont_index,
    std::map<SYMBOL_TYPE, uint8_t>& continity_for_symbol,
    std::map<uint8_t, SYMBOL_TYPE>& symbol_for_continuity,
    const std::string& sheet_name, uint32_t pointNum)
{
    // need to check for symbol inconsistency here.
    if (continity_for_symbol.count(symbol) == 0) {
        // we haven't seen this symbol->cont_index yet
        continity_for_symbol[symbol] = cont_index;
    }
    else {
        if (continity_for_symbol[symbol] != cont_index) {
            std::stringstream buf;
            buf << "Error, symbol inconsistency on sheet \"" << sheet_name << "\".\n";
            buf << "Symbol " << GetNameForSymbol(symbol)
                << " previously used continuity "
                << (uint32_t)continity_for_symbol[symbol] << " but point " << pointNum
                << " on uses continuity " << (uint32_t)cont_index
                << ", which is used by symbol "
                << GetNameForSymbol(symbol_for_continuity[cont_index]) << ".\n";
            buf << "Try opening this file on CalChart v3.3.5 or earlier.\n";
            throw CC_FileException(buf.str());
        }
    }
    if (symbol_for_continuity.count(cont_index) == 0) {
        symbol_for_continuity[cont_index] = symbol;
    }
    else {
        if (symbol_for_continuity[cont_index] != symbol) {
            std::stringstream buf;
            buf << "Error, symbol inconsistency on sheet \"" << sheet_name << "\".\n";
            buf << "Continuity index " << (uint32_t)cont_index
                << " previously used symbol "
                << GetNameForSymbol(symbol_for_continuity[cont_index])
                << "  but point " << pointNum << " on uses symbol "
                << GetNameForSymbol(symbol) << ".\n";
            buf << "Try opening this file on CalChart v3.3.5 or earlier.\n";
            throw CC_FileException(buf.str());
        }
    }
}

// Constructor for shows 3.3 and ealier.
CC_sheet::CC_sheet(size_t numPoints, std::istream& stream,
    Version_3_3_and_earlier)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mPoints(numPoints)
{
    // Read in sheet name
    // <INGL_NAME><size><string + 1>
    std::vector<uint8_t> data = ReadCheckIDandFillData(stream, INGL_NAME);
    mName = (const char*)&data[0];

    // read in the duration:
    // <INGL_DURA><4><duration>
    auto chunk = ReadCheckIDandSize(stream, INGL_DURA);
    mBeats = chunk;

    // Point positions
    // <INGL_DURA><size><data>
    data = ReadCheckIDandFillData(stream, INGL_POS);
    if (data.size() != size_t(mPoints.size() * 4)) {
        throw CC_FileException("bad POS chunk");
    }
    {
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); ++i) {
            CC_coord c;
            c.x = get_big_word(d);
            d += 2;
            c.y = get_big_word(d);
            d += 2;
            for (unsigned j = 0; j <= CC_point::kNumRefPoints; j++) {
                mPoints[i].SetPos(c, j);
            }
        }
    }

    uint32_t name = ReadLong(stream);
    // read all the reference points
    while (INGL_REFP == name) {
        std::vector<uint8_t> data = FillData(stream);
        if (data.size() != mPoints.size() * 4 + 2) {
            throw CC_FileException("Bad REFP chunk");
        }
        uint8_t* d = &data[0];
        unsigned ref = get_big_word(d);
        d += 2;
        for (unsigned i = 0; i < mPoints.size(); i++) {
            CC_coord c;
            c.x = get_big_word(d);
            d += 2;
            c.y = get_big_word(d);
            d += 2;
            mPoints[i].SetPos(c, ref);
        }
        name = ReadLong(stream);
    }
    // Point symbols
    while (INGL_SYMB == name) {
        std::vector<uint8_t> data = FillData(stream);
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad SYMB chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            mPoints.at(i).SetSymbol((SYMBOL_TYPE)(*(d++)));
        }
        name = ReadLong(stream);
    }
    std::map<SYMBOL_TYPE, uint8_t> continity_for_symbol;
    std::map<uint8_t, SYMBOL_TYPE> symbol_for_continuity;
    bool has_type = false;
    // Point continuity types
    while (INGL_TYPE == name) {
        has_type = true;
        std::vector<uint8_t> data = FillData(stream);
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad TYPE chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            CheckInconsistancy(mPoints[i].GetSymbol(), *(d++), continity_for_symbol,
                symbol_for_continuity, mName, i);
        }
        name = ReadLong(stream);
    }
    // because older calchart files may omit the continuity index, need to check
    // if it isn't used
    if (!has_type) {
        // when a point doesn't have a cont_index, it is assumed to be 0
        for (unsigned i = 0; i < mPoints.size(); i++) {
            CheckInconsistancy(mPoints[i].GetSymbol(), 0, continity_for_symbol,
                symbol_for_continuity, mName, i);
        }
    }
    // Point labels (left or right)
    while (INGL_LABL == name) {
        std::vector<uint8_t> data = FillData(stream);
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad SYMB chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            if (*(d++)) {
                mPoints.at(i).Flip();
            }
        }
        name = ReadLong(stream);
    }
    // Continuity text
    while (INGL_CONT == name) {
        std::vector<uint8_t> data = FillData(stream);
        if (data.size() < 3) // one byte num + two nils minimum
        {
            throw CC_FileException("Bad cont chunk");
        }
        const char* d = (const char*)&data[0];
        if (d[data.size() - 1] != '\0') {
            throw CC_FileException("Bad cont chunk");
        }

        const char* text = d + 1;
        size_t num = strlen(text);
        if (data.size() < num + 3) // check for room for text string
        {
            throw CC_FileException("Bad cont chunk");
        }
        std::string namestr(text);
        text = d + 2 + strlen(text);

        auto symbol_index = GetSymbolForName(namestr);
        if (symbol_index == MAX_NUM_SYMBOLS) {
            throw CC_FileException("No viable symbol for name");
        }
        if (continity_for_symbol.count(symbol_index)) {
            // some point is using this symbol, check to see if it points to the same
            // continuity
            if (continity_for_symbol[symbol_index] != (*d)) {
                std::stringstream buf;
                buf << "Error, continuity inconsistency on sheet " << mName << "\n";
                buf << "Continuity index " << (uint32_t)(*d) << " is symbol "
                    << GetNameForSymbol(symbol_index)
                    << " but points using that symbol refer to continuity index "
                    << (uint32_t)continity_for_symbol[symbol_index] << "\n";
                throw CC_FileException(buf.str());
            }
        }
        std::string textstr(text);
        GetContinuityBySymbol(symbol_index) = CC_continuity{textstr};

        name = ReadLong(stream);
    }
}
// -=-=-=-=-=- LEGACY CODE</end> -=-=-=-=-=-

CC_sheet::CC_sheet(size_t numPoints, const uint8_t* ptr, size_t size,
    Current_version_and_later)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mPoints(numPoints)
{
    // construct the parser handlers
    auto parse_INGL_NAME = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        auto str = (const char*)ptr;
        if (size != (strlen(str) + 1)) {
            throw CC_FileException("Description the wrong size", INGL_NAME);
        }
        sheet->mName = str;
    };
    auto parse_INGL_DURA = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        if (4 != size) {
            throw CC_FileException("Incorrect size", INGL_DURA);
        }
        sheet->mBeats = get_big_long(ptr);
    };
    auto parse_INGL_PNTS = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        for (auto i = 0u; i < sheet->mPoints.size(); ++i) {
            auto this_size = *ptr;
            if (this_size > size) {
                throw CC_FileException("Incorrect size", INGL_PNTS);
            }
            sheet->mPoints[i] = CC_point({ ptr + 1, ptr + 1 + this_size });
            ptr += this_size + 1;
            size -= this_size + 1;
        }
        if (size != 0) {
            throw CC_FileException("Incorrect size", INGL_PNTS);
        }
    };
    auto parse_INGL_ECNT = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        if (size < 2) // one byte num + 1 nil minimum
        {
            throw CC_FileException("Bad cont chunk", INGL_ECNT);
        }
        const char* d = (const char*)ptr;
        if (d[size - 1] != '\0') {
            throw CC_FileException("Cont chunk not NULL terminated", INGL_ECNT);
        }
        const char* text = d + 1;
        size_t num = strlen(text);
        if (size < num + 2) // check for room for text string
        {
            throw CC_FileException("Bad cont chunk", INGL_ECNT);
        }
        SYMBOL_TYPE symbol_index = static_cast<SYMBOL_TYPE>(*d);
        if (symbol_index >= MAX_NUM_SYMBOLS) {
            throw CC_FileException("No viable symbol for name", INGL_ECNT);
        }
        std::string textstr(text);
        sheet->GetContinuityBySymbol(symbol_index) = CC_continuity{textstr};
    };
    auto parse_INGL_CONT = [parse_INGL_ECNT](CC_sheet* sheet, const uint8_t* ptr,
        size_t size) {
        static const std::map<uint32_t, std::function<void(CC_sheet*, const uint8_t*, size_t)> >
            parser = {
                { INGL_ECNT, parse_INGL_ECNT },
            };

        auto table = CalChart::Parser::ParseOutLabels(ptr, ptr + size);
        for (auto& i : table) {
            auto the_parser = parser.find(std::get<0>(i));
            if (the_parser != parser.end()) {
                the_parser->second(sheet, std::get<1>(i), std::get<2>(i));
            }
        }
    };
    auto parse_INGL_PCNT = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        const char* print_name = (const char*)ptr;
        const char* print_cont = print_name + strlen(print_name) + 1;
        if ((strlen(print_name) + 1 + strlen(print_cont) + 1) != size) {
            throw CC_FileException("Bad Print cont chunk", INGL_PCNT);
        }
        sheet->mPrintableContinuity = CC_print_continuity(print_name, print_cont);
    };
    auto parse_INGL_BACK = [](CC_sheet* sheet, const uint8_t* ptr, size_t size) {
        auto num = get_big_long(ptr);
        ptr += 4;
        while (num--) {
            sheet->mBackgroundImages.emplace_back(ptr);
        }
    };

    static const std::map<uint32_t, std::function<void(CC_sheet*, const uint8_t*, size_t)> >
        parser = {
            { INGL_NAME, parse_INGL_NAME }, { INGL_DURA, parse_INGL_DURA },
            { INGL_PNTS, parse_INGL_PNTS }, { INGL_CONT, parse_INGL_CONT },
            { INGL_PCNT, parse_INGL_PCNT }, { INGL_BACK, parse_INGL_BACK },
        };

    auto table = CalChart::Parser::ParseOutLabels(ptr, ptr + size);
    for (auto& i : table) {
        auto the_parser = parser.find(std::get<0>(i));
        if (the_parser != parser.end()) {
            the_parser->second(this, std::get<1>(i), std::get<2>(i));
        }
    }
}

std::vector<uint8_t> CC_sheet::SerializeAllPoints() const
{
    // for each of the points, serialize them.  Don't need to wrap in block
    // because it's not specified that way
    std::vector<uint8_t> result;
    for (const auto& i : mPoints) {
        CalChart::Parser::Append(result, i.Serialize());
    }
    return result;
}

std::vector<uint8_t> CC_sheet::SerializeContinuityData() const
{
    // for each continuity in use, serialize them.
    std::vector<uint8_t> result;
    for (auto& current_symbol : k_symbols) {
        if (ContinuityInUse(current_symbol)) {
            std::vector<uint8_t> continuity;
            CalChart::Parser::Append(continuity,
                static_cast<uint8_t>(current_symbol));
            CalChart::Parser::AppendAndNullTerminate(
                continuity, GetContinuityBySymbol(current_symbol).GetText());
            CalChart::Parser::Append(
                result, CalChart::Parser::Construct_block(INGL_ECNT, continuity));
        }
    }
    return result;
}

std::vector<uint8_t> CC_sheet::SerializePrintContinuityData() const
{
    std::vector<uint8_t> result;
    CalChart::Parser::AppendAndNullTerminate(
        result, mPrintableContinuity.GetPrintNumber());
    CalChart::Parser::AppendAndNullTerminate(
        result, mPrintableContinuity.GetOriginalLine());
    return result;
}

std::vector<uint8_t> CC_sheet::SerializeBackgroundImageData() const
{
    std::vector<uint8_t> result;
    CalChart::Parser::Append(result, static_cast<uint32_t>(mBackgroundImages.size()));
    for (auto&& i : mBackgroundImages) {
        CalChart::Parser::Append(result, i.Serialize());
    }
    return result;
}

std::vector<uint8_t> CC_sheet::SerializeSheetData() const
{
    // SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY,
    // PRINT_CONTINUITY ;

    std::vector<uint8_t> result;
    // Write NAME
    std::vector<uint8_t> tstring;
    CalChart::Parser::AppendAndNullTerminate(tstring, GetName());
    CalChart::Parser::Append(
        result, CalChart::Parser::Construct_block(INGL_NAME, tstring));

    // Write DURATION
    CalChart::Parser::Append(result, CalChart::Parser::Construct_block(
                                         INGL_DURA, uint32_t{ GetBeats() }));

    // Write ALL_POINTS
    CalChart::Parser::Append(result, CalChart::Parser::Construct_block(
                                         INGL_PNTS, SerializeAllPoints()));

    // Write Continuity
    CalChart::Parser::Append(result, CalChart::Parser::Construct_block(
                                         INGL_CONT, SerializeContinuityData()));

    // Write Continuity
    CalChart::Parser::Append(result,
        CalChart::Parser::Construct_block(
                                 INGL_PCNT, SerializePrintContinuityData()));

    // Write Continuity
    CalChart::Parser::Append(result, CalChart::Parser::Construct_block(
                                         INGL_BACK, SerializeBackgroundImageData()));

    return result;
}

// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) ,
// SHEET_DATA , SHEET_END ;
// SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY, [
// PRINT_CONTINUITY ] ;
// SHEET_END          = INGL_END , INGL_SHET ;
std::vector<uint8_t> CC_sheet::SerializeSheet() const
{
    std::vector<uint8_t> result;
    CalChart::Parser::Append(result, CalChart::Parser::Construct_block(
                                         INGL_SHET, SerializeSheetData()));
    return result;
}

CC_sheet::~CC_sheet() {}

// Find point at certain coords
int CC_sheet::FindPoint(Coord x, Coord y, Coord searchBound,
    unsigned ref) const
{
    for (auto i = 0; i < static_cast<int>(mPoints.size()); i++) {
        CC_coord c = GetPosition(i, ref);
        if (((x + searchBound) >= c.x) && ((x - searchBound) <= c.x) && ((y + searchBound) >= c.y) && ((y - searchBound) <= c.y)) {
            return i;
        }
    }
    return -1;
}

SelectionList CC_sheet::MakeSelectPointsBySymbol(SYMBOL_TYPE i) const
{
    SelectionList select;
    for (auto j = 0; j < static_cast<int>(mPoints.size()); j++) {
        if (mPoints.at(j).GetSymbol() == i) {
            select.insert(j);
        }
    }
    return select;
}

std::vector<CC_point> CC_sheet::NewNumPointsPositions(int num, int columns, const CC_coord& new_march_position) const
{
    std::vector<CC_point> newpts(mPoints.begin(), mPoints.begin() + std::min<size_t>(mPoints.size(), num));
    auto c = new_march_position;
    auto col = 0;
    auto num_left = num - newpts.size();
    while (num_left--) {
        newpts.push_back(c);
        ++col;
        c.x += Int2Coord(2);
        if (col >= columns) {
            c.x = new_march_position.x;
            c.y += Int2Coord(2);
            col = 0;
        }
    }
    return newpts;
}

void CC_sheet::DeletePoints(SelectionList const& sl)
{
    for (auto iter = sl.rbegin(); iter != sl.rend(); ++iter) {
        mPoints.erase(mPoints.begin() + *iter);
    }
}

std::vector<CC_point> CC_sheet::RemapPoints(const std::vector<size_t>& table) const
{
    if (mPoints.size() != table.size()) {
        throw std::runtime_error("wrong size for Relabel");
    }
    std::vector<CC_point> newpts(mPoints.size());
    for (size_t i = 0; i < newpts.size(); i++) {
        newpts.at(i) = mPoints.at(table.at(i));
    }
    return newpts;
}

const CC_continuity& CC_sheet::GetContinuityBySymbol(SYMBOL_TYPE i) const
{
    return mAnimationContinuity.at(i);
}

CC_continuity& CC_sheet::GetContinuityBySymbol(SYMBOL_TYPE i)
{
    return mAnimationContinuity.at(i);
}

void CC_sheet::SetContinuityText(SYMBOL_TYPE which, const std::string& text)
{
    GetContinuityBySymbol(which) = CC_continuity{text};
}

bool CC_sheet::ContinuityInUse(SYMBOL_TYPE idx) const
{
    // is any point using this symbol?
    for (auto& point : mPoints) {
        if (point.GetSymbol() == idx) {
            return true;
        }
    }
    // otherwise, is the text set.
    return !GetContinuityBySymbol(idx).GetText().empty();
}

std::string CC_sheet::GetName() const { return mName; }

void CC_sheet::SetName(const std::string& newname) { mName = newname; }

std::string CC_sheet::GetNumber() const
{
    return mPrintableContinuity.GetPrintNumber();
}

std::string CC_sheet::GetRawPrintContinuity() const
{
    return mPrintableContinuity.GetOriginalLine();
}

unsigned short CC_sheet::GetBeats() const { return mBeats; }

void CC_sheet::SetBeats(unsigned short b) { mBeats = b; }

// Get position of point
CC_coord CC_sheet::GetPosition(unsigned i, unsigned ref) const
{
    return mPoints[i].GetPos(ref);
}

// Set position of point and all refs
void CC_sheet::SetAllPositions(const CC_coord& val, unsigned i)
{
    for (unsigned j = 0; j <= CC_point::kNumRefPoints; j++) {
        mPoints[i].SetPos(val, j);
    }
}

// Set position of point
void CC_sheet::SetPosition(const CC_coord& val, unsigned i, unsigned ref)
{
    unsigned j;
    if (ref == 0) {
        for (j = 1; j <= CC_point::kNumRefPoints; j++) {
            if (mPoints[i].GetPos(j) == mPoints[i].GetPos(0)) {
                mPoints[i].SetPos(val, j);
            }
        }
        mPoints[i].SetPos(val);
    }
    else {
        mPoints[i].SetPos(val, ref);
    }
}

/* This is the format for each sheet:
 * %%str      where str is the string printed for the stuntsheet number
 * normal ascii text possibly containing the following codes:
 * \bs \be \is \ie for bold start, bold end, italics start, italics end
 * \po plainman
 * \pb backslashman
 * \ps slashman
 * \px xman
 * \so solidman
 * \sb solidbackslashman
 * \ss solidslashman
 * \sx solidxman
 * a line may begin with these symbols in order: <>~
 * < don't print continuity on individual sheets
 * > don't print continuity on master sheet
 * ~ center this line
 * also, there are three tab stops set for standard continuity format
 */

void CC_sheet::SetPrintableContinuity(const std::string& name,
    const std::string& lines)
{
    mPrintableContinuity = CC_print_continuity(name, lines);
}

CC_textline_list CC_sheet::GetPrintableContinuity() const
{
    return mPrintableContinuity.GetChunks();
}

const CC_point& CC_sheet::GetPoint(unsigned i) const { return mPoints[i]; }

CC_point& CC_sheet::GetPoint(unsigned i) { return mPoints[i]; }

std::vector<CC_point> CC_sheet::GetPoints() const { return mPoints; }

JSONElement CC_sheet::toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, const AnimateSheet& compiledSheet) const
{
    JSONElement newViewerObject = JSONElement::makeNull();
    toOnlineViewerJSON(newViewerObject, sheetNum, dotLabels, compiledSheet);
    return newViewerObject;
}

void CC_sheet::toOnlineViewerJSON(JSONElement& dest, unsigned sheetNum, std::vector<std::string> dotLabels, const AnimateSheet& compiledSheet) const
{
    JSONDataObjectAccessor sheetObjectAccessor = dest = JSONElement::makeObject();

    // Create a skeleton for the JSON representation of a sheet
    sheetObjectAccessor["label"] = std::to_string(sheetNum);
    sheetObjectAccessor["field_type"] = "college";
    sheetObjectAccessor["dot_types"] = JSONElement::makeArray();
    sheetObjectAccessor["dot_labels"] = JSONElement::makeObject();
    sheetObjectAccessor["continuities"] = JSONElement::makeObject();
    sheetObjectAccessor["beats"] = mBeats;
    sheetObjectAccessor["movements"] = JSONElement::makeObject();

    JSONDataObjectAccessor dotTypeAssignmentsAccessor = sheetObjectAccessor["dot_labels"];

    // Iterate through all dots and collect the unique dot types discovered while doing so
    // As we discover the dot type of each dot, record the dot type assigment inside of 'dot_labels'
    std::set<std::string> uniqueDotTypes;
    for (unsigned i = 0; i < mPoints.size(); i++) {
        std::string symbolName = ToOnlineViewer::symbolName(mPoints[i].GetSymbol());
        uniqueDotTypes.insert(symbolName);
        dotTypeAssignmentsAccessor[dotLabels[i]] = symbolName;
    }

    JSONDataArrayAccessor dotTypesAccessor = sheetObjectAccessor["dot_types"];
    JSONDataObjectAccessor continuitiesAccessor = sheetObjectAccessor["continuities"];

    // In 'dot_types', list all unique dot types that are used in this sheet
    // In 'continuities', map a dot type to its printed continuity
    for (auto iter = uniqueDotTypes.begin(); iter != uniqueDotTypes.end(); iter++) {
        dotTypesAccessor->pushBack(JSONElement::makeString(*iter));

        continuitiesAccessor[*iter] = JSONElement::makeArray();
        JSONDataArrayAccessor contArrayAccessor = continuitiesAccessor[*iter];

        contArrayAccessor->pushBack("(MANUAL) first continuity instruction goes here for SS" + std::to_string(sheetNum)); // TODO; add printed continuities to viewer file manually for now
        contArrayAccessor->pushBack("(MANUAL) second instruction");
        contArrayAccessor->pushBack("(MANUAL) third instruction...");
    }

    JSONDataObjectAccessor movementsAccessor = sheetObjectAccessor["movements"];

    // In 'movements', make a series of commands to describe how a point should be animated over time in the Online Viewer
    for (unsigned ptIndex = 0; ptIndex < mPoints.size(); ptIndex++) {
        JSONDataArrayAccessor pointMovementsAccessor = movementsAccessor[dotLabels[ptIndex]] = JSONElement::makeArray();

        AnimatePoint currPos(mPoints[ptIndex].GetPos().x, mPoints[ptIndex].GetPos().y);

        for (auto commandIter = compiledSheet.GetCommandsBegin(ptIndex); commandIter != compiledSheet.GetCommandsEnd(ptIndex); commandIter++) {

            pointMovementsAccessor->pushBack(JSONElement::makeNull());
            (*commandIter)->toOnlineViewerJSON(pointMovementsAccessor->back(), currPos);

            (*commandIter)->ApplyForward(currPos);
        }
    }
}

void CC_sheet::SetPoints(const std::vector<CC_point>& points) { mPoints = points; }

// -=-=-=-=-=-=- Unit Tests -=-=-=-=-=-=-=-
#include <assert.h>
using namespace CalChart::Parser;

void CC_sheet::CC_sheet_round_trip_test()
{
    {
        auto blank_sheet = CC_sheet::CC_sheet(0);
        auto blank_sheet_data = blank_sheet.SerializeSheet();
        // need to pull out the sheet data
        auto table = CalChart::Parser::ParseOutLabels(blank_sheet_data.data(),
            blank_sheet_data.data() + blank_sheet_data.size());
        assert(table.size() == 1);
        assert(std::get<0>(table.front()) == INGL_SHET);
        auto re_read_sheet = CC_sheet::CC_sheet(0, std::get<1>(table.front()),
            std::get<2>(table.front()),
            Current_version_and_later());
        auto re_read_sheet_data = re_read_sheet.SerializeSheet();
        bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(),
                                                                                    re_read_sheet_data.begin());
        assert(is_equal);
    }
    {
        auto blank_sheet = CC_sheet::CC_sheet(0, "new_sheet");
        auto blank_sheet_data = blank_sheet.SerializeSheet();
        // need to pull out the sheet data
        auto table = CalChart::Parser::ParseOutLabels(blank_sheet_data.data(),
            blank_sheet_data.data() + blank_sheet_data.size());
        assert(table.size() == 1);
        assert(std::get<0>(table.front()) == INGL_SHET);
        auto re_read_sheet = CC_sheet::CC_sheet(0, std::get<1>(table.front()),
            std::get<2>(table.front()),
            Current_version_and_later());
        auto re_read_sheet_data = re_read_sheet.SerializeSheet();
        bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(),
                                                                                    re_read_sheet_data.begin());
        assert(is_equal);
    }
    {
        auto blank_sheet = CC_sheet::CC_sheet(1, "new_sheet");
        blank_sheet.SetName("new_name");
        blank_sheet.SetPosition(CC_coord(10, 10), 0);
        blank_sheet.SetPosition(CC_coord(20, 10), 0, 1);
        blank_sheet.SetPosition(CC_coord(30, 40), 0, 2);
        blank_sheet.SetPosition(CC_coord(52, 50), 0, 3);
        blank_sheet.SetBeats(13);
        blank_sheet.GetContinuityBySymbol(SYMBOL_PLAIN) = CC_continuity{"continuity test"};
        blank_sheet.mPrintableContinuity = CC_print_continuity{
            "number 1", "duuuude, writing this testing is boring"
        };
        auto blank_sheet_data = blank_sheet.SerializeSheet();
        // need to pull out the sheet data
        auto table = CalChart::Parser::ParseOutLabels(blank_sheet_data.data(),
            blank_sheet_data.data() + blank_sheet_data.size());
        assert(table.size() == 1);
        assert(std::get<0>(table.front()) == INGL_SHET);
        auto re_read_sheet = CC_sheet::CC_sheet(1, std::get<1>(table.front()),
            std::get<2>(table.front()),
            Current_version_and_later());
        auto re_read_sheet_data = re_read_sheet.SerializeSheet();
        bool is_equal = blank_sheet_data.size() == re_read_sheet_data.size() && std::equal(blank_sheet_data.begin(), blank_sheet_data.end(),
                                                                                    re_read_sheet_data.begin());
        //		auto mismatch_at = std::mismatch(blank_sheet_data.begin(),
        // blank_sheet_data.end(), re_read_sheet_data.begin());
        //		std::cout<<"mismatch at
        //"<<std::distance(blank_sheet_data.begin(),
        // mismatch_at.first)<<"\n";
        assert(is_equal);
    }
}

void CC_sheet_UnitTests() { CC_sheet::CC_sheet_round_trip_test(); }

std::vector<calchart_core::ImageData> const& CC_sheet::GetBackgroundImages() const
{
    return mBackgroundImages;
}

void CC_sheet::AddBackgroundImage(calchart_core::ImageData const& image, size_t where)
{
    auto insert_point = mBackgroundImages.begin() + std::min(where, mBackgroundImages.size());
    mBackgroundImages.insert(insert_point, image);
}

void CC_sheet::RemoveBackgroundImage(size_t which)
{
    if (which < mBackgroundImages.size()) {
        mBackgroundImages.erase(mBackgroundImages.begin() + which);
    }
}

void CC_sheet::MoveBackgroundImage(size_t which, int left, int top, int scaled_width, int scaled_height)
{
    if (which < mBackgroundImages.size()) {
        mBackgroundImages.at(which).left = left;
        mBackgroundImages.at(which).top = top;
        mBackgroundImages.at(which).scaled_width = scaled_width;
        mBackgroundImages.at(which).scaled_height = scaled_height;
    }
}
