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
#include <gtest/gtest.h>
#include <iostream>
//#include <numeric>
#include <memory>
#include <string>
#include <sstream>
#include <system_error>
#include <vector>

#include <spdlog/spdlog.h>

#include <chartdir.h>
#include <gmock/gmock.h>

using namespace std::string_literals;

namespace fs = std::filesystem;

#include <range/v3/algorithm/for_each.hpp>

using namespace testing;

#include "DDecDouble.h"
#include "p_f_column.h"
#include "p_f_data.h"

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

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, P_F_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
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
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

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
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

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
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1120);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 4);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 
    auto col = std::make_unique<P_F_Column>(10, 1);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1140);
    EXPECT_EQ(col->GetBottom(), 1130);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 9);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}


class ColumnFunctionality10X3 : public Test
{

};

TEST_F(ColumnFunctionality10X3, Constructors)
{
   P_F_Column col;

   ASSERT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X3, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    P_F_Column col{10, 3};
    
    auto a_value = prices.begin();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecDouble{*a_value});
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, P_F_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecDouble{*(++a_value)});
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecDouble(*a_value));
    }
    EXPECT_EQ(status.first, P_F_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X3, ProcessFirstHalfOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};

    auto col = std::make_unique<P_F_Column>(10, 3);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1120);
    EXPECT_EQ(col->GetBottom(), 1100);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 2);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}

TEST_F(ColumnFunctionality10X3, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<P_F_Column>(10, 3);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1150);
    EXPECT_EQ(col->GetBottom(), 1110);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 3);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}

class ColumnFunctionality10X5 : public Test
{

};

TEST_F(ColumnFunctionality10X5, Constructors)
{
   P_F_Column col(10, 5);

   ASSERT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X5, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<P_F_Column>(10, 5);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_up);
    EXPECT_EQ(col->GetTop(), 1150);
    EXPECT_EQ(col->GetBottom(), 1100);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 1);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}

class ColumnFunctionality10X2 : public Test
{

};

TEST_F(ColumnFunctionality10X2, Constructors)
{
   P_F_Column col(10, 2);

   ASSERT_EQ(col.GetDirection(), P_F_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    auto col = std::make_unique<P_F_Column>(10, 2);

    std::vector<P_F_Column> columns;

    for (auto price : prices)
    {
//        std::cout << "price: " << price << '\n';
        auto [status, new_col] = col->AddValue(DprDecimal::DDecDouble(price));
//        std::cout << " status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        if (status == P_F_Column::Status::e_reversal)
        {
            auto* save_col = col.get();         // non-owning access
            columns.push_back(*save_col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col->AddValue(DprDecimal::DDecDouble(price)).first;
//            std::cout << "new column status: " << status << " top: " << col->GetTop() << " bottom: " << col->GetBottom() << " direction: " << col->GetDirection() << '\n';
        }
    }

    EXPECT_EQ(col->GetDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(col->GetTop(), 1140);
    EXPECT_EQ(col->GetBottom(), 1130);
    EXPECT_EQ(col->GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 6);

    for (const auto& a_col : columns)
    {
        std::cout << "bottom: " << a_col.GetBottom() << " top: " << a_col.GetTop() << " direction: " << a_col.GetDirection() << (a_col.GetHadReversal() ? " one step back reversal" : "") << '\n';
    }
    std::cout << "bottom: " << col->GetBottom() << " top: " << col->GetTop() << " direction: " << col->GetDirection() << (col->GetHadReversal() ? " one step back reversal" : "") << '\n';
}

class ChartFunctionality10X2 : public Test
{

};

TEST_F(ChartFunctionality10X2, Constructors)
{
   P_F_Data chart("GOOG", 10, 2);

   ASSERT_EQ(chart.GetCurrentDirection(), P_F_Column::Direction::e_unknown);

}

TEST_F(ChartFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    std::string data = {"1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 "};
    data += "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::stringstream prices{data, std::ios_base::in}; 

    P_F_Data chart("GOOG", 10, 2);
    chart.LoadData<int32_t>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    chart.ExportData(&std::cout);
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalData)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    P_F_Data chart("AAPL", 2, 2);
    chart.LoadData<DDecDouble>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    chart.ExportData(&std::cout);
}

class PlotChartsWithChartDirector : public Test
{

};

TEST_F(PlotChartsWithChartDirector, Plot10X3Chart)
{
    std::string data = {"1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 "};
    data += "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::stringstream prices{data, std::ios_base::in}; 

    P_F_Data chart("GOOG", 10, 3);
    chart.LoadData<int32_t>(&prices);

    EXPECT_EQ(chart.GetCurrentDirection(), P_F_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    chart.ExportData(&std::cout);

    XYChart* c = new XYChart(600, 350);

    // Set the plotarea at (50, 25) and of size 500 x 250 pixels. Enable both the horizontal and
    // vertical grids by setting their colors to grey (0xc0c0c0)
    c->setPlotArea(50, 25, 500, 250)->setGridColor(0xc0c0c0, 0xc0c0c0);

    // Add a title to the chart
    c->addTitle("Universal Stock Index on Jan 2001");

    // Add a custom text at (50, 25) (the upper left corner of the plotarea). Use 12pt Arial
    // Bold/blue (4040c0) as the font.
    c->addText(50, 25, "(c) Global XYZ ABC Company", "Arial Bold", 12, 0x4040c0);

    // Add a title to the x axis
    c->xAxis()->setTitle("Jan 2001");

    // Set the labels on the x axis. Rotate the labels by 45 degrees.
//    c->xAxis()->setLabels(StringArray(labels, labels_size))->setFontAngle(45);

    // Add a title to the y axis
    c->yAxis()->setTitle("Universal Stock Index");

    // Draw the y axis on the right hand side of the plot area
    c->setYAxisOnRight(true);

    // Add a CandleStick layer to the chart using green (00ff00) for up candles and red (ff0000) for
    // down candles
//    CandleStickLayer* layer = c->addCandleStickLayer(DoubleArray(highData, highData_size),
//        DoubleArray(lowData, lowData_size), DoubleArray(openData, openData_size), DoubleArray(
//        closeData, closeData_size), 0x00ff00, 0xff0000);

    // Set the line width to 2 pixels
//    layer->setLineWidth(2);

    // Output the chart
    c->makeChart("/tmp/candlestick.png");

    //free up resources
    delete c;
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
