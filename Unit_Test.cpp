// =====================================================================================
//
//       Filename:  Unit_Test.cpp
//
//    Description:  Driver program for Unit tests
//
//        Version:  1.0
//        Created:  2021-07-24 10:14 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  David P. Riedel (dpr), driedel@cox.net
//        License:  GNU General Public License v3
//        Company:
//
// =====================================================================================

    /* This file is part of Point and Figure. */

    /* Extractor_Markup is free software: you can redistribute it and/or modify */
    /* it under the terms of the GNU General Public License as published by */
    /* the Free Software Foundation, either version 3 of the License, or */
    /* (at your option) any later version. */

    /* Extractor_Markup is distributed in the hope that it will be useful, */
    /* but WITHOUT ANY WARRANTY; without even the implied warranty of */
    /* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
    /* GNU General Public License for more details. */

    /* You should have received a copy of the GNU General Public License */
    /* along with Extractor_Markup.  If not, see <http://www.gnu.org/licenses/>. */


// =====================================================================================
//        Class:
//  Description:
// =====================================================================================


//#include <algorithm>
//#include <chrono>
//#include <cstdint>
#include <filesystem>
//#include <fstream>
#include <gtest/gtest.h>
#include <iostream>
//#include <numeric>
#include <memory>
#include <string>
#include <system_error>
#include <vector>

#include <spdlog/spdlog.h>

#include <gmock/gmock.h>

using namespace std::string_literals;

namespace fs = std::filesystem;

#include <range/v3/algorithm/for_each.hpp>

using namespace testing;

#include "DDecDouble.h"
#include "p_f_column.h"

using namespace DprDecimal;

// some specific files for Testing.

// This ctype facet does NOT classify spaces and tabs as whitespace
// from cppreference example

struct line_only_whitespace : std::ctype<char>
{
    static const mask* make_table()
    {
        // make a copy of the "C" locale table
        static std::vector<mask> v(classic_table(), classic_table() + table_size);
        v['\t'] &= ~space;      // tab will not be classified as whitespace
        v[' '] &= ~space;       // space will not be classified as whitespace
        return &v[0];
    }
    explicit line_only_whitespace(std::size_t refs = 0) : ctype(make_table(), false, refs) {}
};


class DecimalBasicFunctionality : public Test
{

};

TEST_F(DecimalBasicFunctionality, Constructors)
{

    DDecDouble x1;
    DDecDouble x2{"5"};
    DDecDouble x3{"1234.3"s};

    DDecDouble x4{1.25678, 3};

    DDecDouble x5{1.257, 3};

    DDecDouble x6{5.0, 1};

    EXPECT_EQ(x2, 5);
    EXPECT_EQ(x3, 1234.3);
    EXPECT_EQ(x4, 1.257);
    EXPECT_EQ(x4, x5);
    
    // test that this works
    EXPECT_EQ(x2, x6);

}

TEST_F(DecimalBasicFunctionality, SimpleArithmetic)
{
    DDecDouble x1{5};
    auto x1_result = x1 + 5;
    EXPECT_EQ(x1_result, 10);

    DDecDouble x2{1.23457, 5};
    auto x2_result = x2 * 2;
    EXPECT_EQ(x2_result, 2.46914);
    EXPECT_TRUE(x2_result == 2.46914);
}

class ColumnFunctionality10X1 : public Test
{

};

TEST_F(ColumnFunctionality10X1, Constructors)
{
   P_F_Column col;

   ASSERT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    P_F_Column col{10, 1};
    
    auto a_value = prices.begin();

    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X1, ContinueUntilFirstReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    P_F_Column col{10, 1};

    P_F_Column::Status status;
    ranges::for_each(prices, [&col, &status](auto price) { status = col.AddValue(DprDecimal::DDecDouble(price)).first; });
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1100);
    ASSERT_EQ(status, P_F_Column::Status::e_reversal);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    auto col = std::make_unique<P_F_Column>(10, 1);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
        auto [status, value] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.release();
            col = std::make_unique<P_F_Column>(save_col->GetBoxsize(), save_col->GetReversalboxes(),
                    save_col->GetDirection() == P_F_Column::Direction::e_up ? P_F_Column::Direction::e_down : P_F_Column::Direction::e_up, value, value);
            columns.push_back(*save_col);

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
        }
    }

    EXPECT_EQ(columns.back().GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(columns.back().GetTop(), 1130);
    EXPECT_EQ(columns.back().GetBottom(), 1100);

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1120);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedByOneStepBack)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139}; 
    auto col = std::make_unique<P_F_Column>(10, 1);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
        auto [status, value] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.release();
            col = std::make_unique<P_F_Column>(save_col->GetBoxsize(), save_col->GetReversalboxes(),
                    save_col->GetDirection() == P_F_Column::Direction::e_up ? P_F_Column::Direction::e_down : P_F_Column::Direction::e_up, value, value);
            columns.push_back(*save_col);

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1130);
    EXPECT_EQ(col->GetBottom(), 1120);
    EXPECT_EQ(col->GetHadReversal(), true);
    EXPECT_EQ(columns.size() + 1, 2);
}
TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedBySeriesOfOneStepBacks)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111}; 
    auto col = std::make_unique<P_F_Column>(10, 1);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
        std::cout << "price: " << price << '\n';
        auto [status, value] = col->AddValue(DprDecimal::DDecDouble(price));
        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.release();
            col = std::make_unique<P_F_Column>(save_col->GetBoxsize(), save_col->GetReversalboxes(),
                    save_col->GetDirection() == P_F_Column::Direction::e_up ? P_F_Column::Direction::e_down : P_F_Column::Direction::e_up, value, value);
            columns.push_back(*save_col);

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1120);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 4);
}



/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  InitLogging
 *  Description:  
 * =====================================================================================
 */
void InitLogging ()
{
    spdlog::set_level(spdlog::level::debug);
//    logging::core::get()->set_filter
//    (
//        logging::trivial::severity >= logging::trivial::trace
//    );
}		/* -----  end of function InitLogging  ----- */

int main(int argc, char** argv)
{

    InitLogging();

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
