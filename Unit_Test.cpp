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
#include <fstream>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <vector>
//#include <numeric>

#include <gmock/gmock.h>

#include <spdlog/spdlog.h>

using namespace std::literals::chrono_literals;
using namespace std::string_literals;
namespace fs = std::filesystem;

#include <range/v3/algorithm/for_each.hpp>

// for boost websocket testing

using namespace testing;

#include "DDecDouble.h"
#include "PF_Column.h"
#include "PF_Chart.h"
#include "LiveStream.h"

using namespace DprDecimal;

// some specific files for Testing.

namespace boost
{
    // these functions are declared in the library headers but left to the user to define.
    // so here they are...
    //
    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  assertion_failed_mgs
     *  Description:  
     *         defined in boost header but left to us to implement.
     * =====================================================================================
     */

    void assertion_failed_msg (char const* expr, char const* msg, char const* function, char const* file, long line)
    {
        throw std::invalid_argument(fmt::format("\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {} \nassertion msg: {}",
                    expr, function, file, line,  msg));
    }		/* -----  end of function assertion_failed_mgs  ----- */

    /* 
     * ===  FUNCTION  ======================================================================
     *         Name:  assertion_failed
     *  Description:  
     * =====================================================================================
     */
    void assertion_failed (char const* expr, char const* function, char const* file, long line )
    {
        throw std::invalid_argument(fmt::format("\n*** Assertion failed *** test: {} in function: {} from file: {} at line: {}",
                    expr, function, file, line));
    }		/* -----  end of function assertion_failed  ----- */
} /* end namespace boost */
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
   PF_Column col;

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    PF_Column col{10, 1};
    
    auto a_value = prices.begin();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X1, ContinueUntilFirstReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    PF_Column col{10, 1};

    PF_Column::Status status;
    ranges::for_each(prices, [&col, &status](auto price) { status = col.AddValue(DprDecimal::DDecDouble(price)).first; });
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1100);
    ASSERT_EQ(status, PF_Column::Status::e_reversal);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    auto col = std::make_unique<PF_Column>(10, 1);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
        }
    }

    EXPECT_EQ(columns.back().GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(columns.back().GetTop(), 1130);
    EXPECT_EQ(columns.back().GetBottom(), 1100);

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1120);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedByOneStepBack)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139}; 
    auto col = std::make_unique<PF_Column>(10, 1);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1130);
    EXPECT_EQ(col->GetBottom(), 1120);
    EXPECT_EQ(col->GetHadReversal(), true);
    EXPECT_EQ(columns.size() + 1, 2);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedBySeriesOfOneStepBacks)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111}; 
    auto col = std::make_unique<PF_Column>(10, 1);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1120);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 4);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 
    auto col = std::make_unique<PF_Column>(10, 1);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1140);
    EXPECT_EQ(col->GetBottom(), 1130);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 9);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

class ColumnFunctionalityFractionalBoxes10X1 : public Test
{

};

TEST_F(ColumnFunctionalityFractionalBoxes10X1, Constructors)
{
   PF_Column col{10, 1, PF_Column::FractionalBoxes::e_fractional};

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionalityFractionalBoxes10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<double> prices = {1100.4, 1105.9, 1110.3, 1112.2, 1118.7, 1120.6}; 
    PF_Column col{10, 1, PF_Column::FractionalBoxes::e_fractional};
    
    auto a_value = prices.begin();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);

    std:: cout << "col: " << col << '\n';
}


class ColumnFunctionality10X3 : public Test
{

};

TEST_F(ColumnFunctionality10X3, Constructors)
{
   PF_Column col;

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X3, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    PF_Column col{10, 3};
    
    auto a_value = prices.begin();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X3, ProcessFirstHalfOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};

    auto col = std::make_unique<PF_Column>(10, 3);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1100);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 2);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X3, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<PF_Column>(10, 3);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1150);
    EXPECT_EQ(col->GetBottom(), 1110);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 3);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

class ColumnFunctionality10X5 : public Test
{

};

TEST_F(ColumnFunctionality10X5, Constructors)
{
   PF_Column col(10, 5);

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X5, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<PF_Column>(10, 5);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1150);
    EXPECT_EQ(col->GetBottom(), 1100);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 1);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

class ColumnFunctionality10X2 : public Test
{

};

TEST_F(ColumnFunctionality10X2, Constructors)
{
   PF_Column col(10, 2);

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<PF_Column>(10, 2);

    std::vector<PF_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1140);
    EXPECT_EQ(col->GetBottom(), 1130);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 6);

    for (const auto& a_col : columns)
    {
        std::cout << a_col << '\n';
    }
    std::cout << col << '\n';
}

class ChartFunctionality10X2 : public Test
{

};

TEST_F(ChartFunctionality10X2, Constructors)
{
   PF_Chart chart("GOOG", 10, 2);

   ASSERT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ChartFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    std::string data = {"1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 "};
    data += "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::stringstream prices{data, std::ios_base::in}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData<int32_t>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalData)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

//    PF_Chart chart("AAPL", 2, 2, PF_Column::FractionalBoxes::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData<DDecDouble>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 62);

    EXPECT_EQ(chart[61].GetTop(), 146);
    EXPECT_EQ(chart[61].GetBottom(), 144);

//    std::cout << chart << '\n';
}

class PlotChartsWithChartDirector : public Test
{

};

TEST_F(PlotChartsWithChartDirector, Plot10X2Chart)
{
    if (fs::exists("/tmp/candlestick.svg"))
    {
        fs::remove("/tmp/candlestick.svg");
    }
    std::string data = {"1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 "};
    data += "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::stringstream prices{data, std::ios_base::in}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData<int32_t>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    std::cout << chart << '\n';

    chart.ConstructChartAndWriteToFile("/tmp/candlestick.svg");

    ASSERT_TRUE(fs::exists("/tmp/candlestick.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalData)
{
    if (fs::exists("/tmp/candlestick2.svg"))
    {
        fs::remove("/tmp/candlestick2.svg");
    }
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData<DDecDouble>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 62);

    EXPECT_EQ(chart[61].GetTop(), 146);
    EXPECT_EQ(chart[61].GetBottom(), 144);

//    std::cout << chart << '\n';

    chart.ConstructChartAndWriteToFile("/tmp/candlestick2.svg");
    
    ASSERT_TRUE(fs::exists("/tmp/candlestick2.svg"));
}

class WebSocketSynchronous : public Test
{
    std::string LoadApiKey(std::string file_name)
    {
        if (! fs::exists(file_name))
        {
            throw std::runtime_error("Can't find key file.");
        }
        std::ifstream key_file(file_name);
        std::string result;
        key_file >> result;
        return result;
    }
public:

    const std::string api_key = LoadApiKey("./tiingo_key.dat");

};

TEST_F(WebSocketSynchronous, DISABLED_ConnectAndDisconnect)
{
    LiveStream quotes{"api.tiingo.com", "443", "/iex", api_key, "spy,uso,rsp"};
    quotes.Connect();
    bool time_to_stop = false;
    auto the_task = std::async(std::launch::async, &LiveStream::StreamData, &quotes, &time_to_stop);
	std::this_thread::sleep_for(10s);
    time_to_stop = true;
	the_task.get();
//    ASSERT_EXIT((the_task.get()),::testing::KilledBySignal(SIGINT),".*");
    quotes.Disconnect();

    for (const auto & value: quotes)
    {
        std::cout << value << '\n';
    }
    ASSERT_TRUE(! quotes.empty());         // we need an actual test here
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
