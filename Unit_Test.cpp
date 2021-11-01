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
#include <chrono>
//#include <cstdint>

//#include <chrono>
#include <functional>
#include <filesystem>
#include <fstream>
#include <future>
//#include <iostream>
//#include <memory>
#include <sstream>
#include <string>
#include <string_view>
//#include <system_error>
#include <thread>
//#include <numeric>

#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/zip_with.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/reverse.hpp>

/* #include <gmock/gmock.h> */
#include <gtest/gtest.h>

#include <date/date.h>
#include <spdlog/spdlog.h>

using namespace std::literals::chrono_literals;
using namespace date::literals;
using namespace std::string_literals;
namespace fs = std::filesystem;


// for boost websocket testing

using namespace testing;

#include "DDecQuad.h"
#include "PF_Column.h"
#include "PF_Chart.h"
#include "Tiingo.h"

#include "utilities.h"

using namespace DprDecimal;

// some specific files for Testing.


class RangeSplitterBasicFunctionality : public Test
{

};

TEST_F(RangeSplitterBasicFunctionality, Test1)
{
    const std::string data = "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 " \
    "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    auto values = split_string<std::string_view>(data, ' ');

    auto items = rng_split_string<std::string_view>(data, ' ');

    // thee loop below demonstrates that this is a 'lazy' split 
//    int i = 0;
//    for(const auto& item : items)
//    {
//        std::cout <<  "i: " << ++i << " " << item << '\n';
//    }

    EXPECT_EQ(values.size(), ranges::distance(items));

    std::vector<std::string_view> values2;

    ranges::for_each(items, [&values2](const auto& x) { values2.push_back(x); });

    ASSERT_EQ(values, values2);
};

class BusinessDateRange : public Test
{

};

TEST_F(BusinessDateRange, WithinSingleWeek)
{
    date::year_month_day start_here{2021_y/date::October/date::Friday[1]};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Down);

    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, date::year_month_day{2021_y/date::September/27});

}

TEST_F(BusinessDateRange, SpanAWeek)
{
    date::year_month_day start_here{2021_y/date::October/date::Friday[1]};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Up);

    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, date::year_month_day{2021_y/date::October/7});

}

TEST_F(BusinessDateRange, SpanAWeekAndAMonth)
{
    date::year_month_day start_here{2021_y/date::September/22};

    auto result = ConstructeBusinessDayRange(start_here, 12, UpOrDown::e_Up);

    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.second, date::year_month_day{2021_y/date::October/7});

    start_here = date::year_month_day{2021_y/date::October/7};

    result = ConstructeBusinessDayRange(start_here, 14, UpOrDown::e_Down);

    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, date::year_month_day{2021_y/date::September/20});

}

class DecimalBasicFunctionality : public Test
{

};

TEST_F(DecimalBasicFunctionality, Constructors)
{

    DDecQuad x1;
    DDecQuad x2{"5"};
    DDecQuad x3{"1234.3"s};

    DDecQuad x4{1.25678, 3};

    DDecQuad x5{1.257, 3};

    DDecQuad x6{5.0, 1};

    EXPECT_EQ(x2, 5);
    EXPECT_EQ(x3, 1234.3);
    EXPECT_EQ(x4, 1.257);
    EXPECT_EQ(x4, x5);
    
    // test that this works
    EXPECT_EQ(x2, x6);

}

TEST_F(DecimalBasicFunctionality, SimpleArithmetic)
{
    DDecQuad x1{5};
    auto x1_result = x1 + 5;
    EXPECT_EQ(x1_result, 10);

    DDecQuad x2{1.23457, 5};
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

TEST_F(ColumnFunctionality10X1, Equality)
{
   PF_Column col1;

   EXPECT_EQ(col1.GetDirection(), PF_Column::Direction::e_unknown);

   PF_Column col2;
   EXPECT_EQ(col1, col2);

   PF_Column col3{5, 5};
   ASSERT_NE(col3, col2);

}

TEST_F(ColumnFunctionality10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    PF_Column col{10, 1};
    
    auto a_value = prices.begin();

    PF_Column::tpt the_time = std::chrono::system_clock::now();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecQuad{*a_value}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecQuad(*a_value), the_time);
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
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1100);
    ASSERT_EQ(status, PF_Column::Status::e_reversal);
}

TEST_F(ColumnFunctionality10X1, ContinueUntilFirstReversalThenJSON)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    PF_Column col{10, 1};

    PF_Column::Status status;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    auto json = col.ToJSON();
//    std::cout << json << '\n';

    EXPECT_EQ(json["box_size"], "10");

    // this is be the way to convert our stored time to a time_point.
    // we use nanoseconds because that is what tiingo provides in its 
    // streaming interface.

    PF_Column::tpt z{std::chrono::nanoseconds{json["start_at"].asInt64()}};
    
    EXPECT_TRUE(z == the_time);

//    std::cout << std::chrono::milliseconds(std::stol(json["start_at"])) << ;\n';

    EXPECT_EQ(json["direction"].asString(), "up");
    EXPECT_EQ(json["top"].asString(), "1130");
    EXPECT_EQ(json["bottom"].asString(), "1100");
    ASSERT_EQ(status, PF_Column::Status::e_reversal);
}

TEST_F(ColumnFunctionality10X1, ConstructValueStoreAsJSONThenConstructCopy)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    PF_Column col{10, 1};

    PF_Column::Status status;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    auto json = col.ToJSON();
//    std::cout << json << '\n';

    PF_Column col2{json};
    EXPECT_EQ(col, col2);
    EXPECT_EQ(col.GetTimeSpan(), col2.GetTimeSpan());
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    auto col = PF_Column{10, 1};

    std::vector<PF_Column> columns;

    PF_Column::tpt the_time = std::chrono::system_clock::now();
    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(columns.back().GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(columns.back().GetTop(), 1130);
    EXPECT_EQ(columns.back().GetBottom(), 1100);

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1120);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedByOneStepBack)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139}; 
    auto col = PF_Column{10, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1120);
    EXPECT_EQ(col.GetHadReversal(), true);
    EXPECT_EQ(columns.size() + 1, 2);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedBySeriesOfOneStepBacks)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111}; 
    auto col = PF_Column{10, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1120);
    EXPECT_EQ(col.GetHadReversal(), false);
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
    auto col = PF_Column{10, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1140);
    EXPECT_EQ(col.GetBottom(), 1130);
    EXPECT_EQ(col.GetHadReversal(), false);
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
    PF_Column::tpt the_time = std::chrono::system_clock::now();
    
    auto a_value = prices.begin();

//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecQuad{*a_value}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecQuad(*a_value), the_time);
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

    PF_Column::tpt the_time = std::chrono::system_clock::now();
//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecQuad{*a_value}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecQuad(*a_value), the_time);
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X3, ProcessFirstHalfOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};

    auto col = PF_Column{10, 3};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
    EXPECT_EQ(col.GetHadReversal(), false);
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

    auto col = PF_Column{10, 3};

    std::vector<PF_Column> columns;

    PF_Column::tpt the_time = std::chrono::system_clock::now();
    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1150);
    EXPECT_EQ(col.GetBottom(), 1110);
    EXPECT_EQ(col.GetHadReversal(), false);
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

    auto col = PF_Column{10, 5};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1150);
    EXPECT_EQ(col.GetBottom(), 1100);
    EXPECT_EQ(col.GetHadReversal(), false);
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

    auto col = PF_Column{10, 2};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1140);
    EXPECT_EQ(col.GetBottom(), 1130);
    EXPECT_EQ(col.GetHadReversal(), false);
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

TEST_F(ChartFunctionality10X2, EmptyChartToJSON)
{
   PF_Chart chart("GOOG", 10, 2);

   EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_unknown);

   auto json = chart.ToJSON();
   std::cout << json << '\n';

   ASSERT_EQ(json["current_direction"].asString(), "unknown");
}

TEST_F(ChartFunctionality10X2, EmptyChartToAndFromJSON)
{
   PF_Chart chart("GOOG", 10, 2);

   EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_unknown);

   auto json = chart.ToJSON();
//   std::cout << json << '\n';

   EXPECT_EQ(json["current_direction"].asString(), "unknown");

   PF_Chart chart2{json};
   ASSERT_EQ(chart, chart2);

}

TEST_F(ChartFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    const std::string data = "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 " \
    "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    auto values = rng_split_string<std::string_view>(data, ' ');

    // make some business days (although, not doing holidays)
    auto dates = ranges::views::generate_n([start_at = date::year_month_day {2015_y/date::March/date::Monday[1]}]()mutable->date::year_month_day
       {
            auto a = start_at;
            auto days = date::sys_days(start_at);
            date::weekday wd{++days};
            if (wd == date::Saturday)
            {
                ++days;
                ++days;
            }
            else if (wd == date::Sunday)
            {
                ++days;
            }
            start_at = date::year_month_day{days};
            return a;
       }, ranges::distance(values));

//    auto sample = dates | ranges::views::take(50);
//    std::cout << sample << '\n';

    auto make_test_data = ranges::views::zip_with([](const date::year_month_day& a_date, std::string_view a_value)
            { std::ostringstream test_data; test_data << a_date << ',' << a_value << '\n'; return test_data.str(); }, dates, values);

//    auto sample2 = make_test_data | ranges::views::take(30);
//    std::cout << sample2 << '\n';

    std::string test_data;

    ranges::for_each(make_test_data, [&test_data](const std::string& new_data){ test_data += new_data; } );

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsInts)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

//    PF_Chart chart("AAPL", 2, 2, PF_Column::FractionalBoxes::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 62);

    EXPECT_EQ(chart[61].GetTop(), 146);
    EXPECT_EQ(chart[61].GetBottom(), 144);

//    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsThenJSON)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

//    PF_Chart chart("AAPL", 2, 2, PF_Column::FractionalBoxes::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    auto json = chart.ToJSON();

    EXPECT_EQ(json["current_direction"].asString(), "down");
    EXPECT_EQ(json["columns"].size(), 61);

    EXPECT_EQ(json["columns"][60]["top"].asString(), "148");
    EXPECT_EQ(json["columns"][60]["bottom"].asString(), "144");

//    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsToJSONFromJSON)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

//    PF_Chart chart("AAPL", 2, 2, PF_Column::FractionalBoxes::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    auto json = chart.ToJSON();

    PF_Chart chart2{json};
    ASSERT_EQ(chart, chart2);

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
    const std::string data = "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 " \
    "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    auto values = rng_split_string<std::string_view>(data, ' ');

    // make some business days (although, not doing holidays)
    auto dates = ranges::views::generate_n([start_at = date::year_month_day {2015_y/date::March/date::Monday[1]}]()mutable->date::year_month_day
       {
            auto a = start_at;
            auto days = date::sys_days(start_at);
            date::weekday wd{++days};
            if (wd == date::Saturday)
            {
                ++days;
                ++days;
            }
            else if (wd == date::Sunday)
            {
                ++days;
            }
            start_at = date::year_month_day{days};
            return a;
       }, ranges::distance(values));

    auto make_test_data = ranges::views::zip_with([](const date::year_month_day& a_date, std::string_view a_value)
            { std::ostringstream test_data; test_data << a_date << ',' << a_value << '\n'; return test_data.str(); }, dates, values);

    std::string test_data;

    ranges::for_each(make_test_data, [&test_data](const std::string& new_data){ test_data += new_data; } );

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

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
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 62);

    EXPECT_EQ(chart[61].GetTop(), 146);
    EXPECT_EQ(chart[61].GetBottom(), 144);

//    std::cout << chart << '\n';

    chart.ConstructChartAndWriteToFile("/tmp/candlestick2.svg");
    
    ASSERT_TRUE(fs::exists("/tmp/candlestick2.svg"));
}

class TiingoATR : public Test
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

TEST_F(TiingoATR, RetrievePreviousData)
{
    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, 14);

    EXPECT_EQ(history.size(), 14);
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0]["date"].asString()), date::year_month_day{2021_y/date::October/7});
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[13]["date"].asString()), date::year_month_day{2021_y/date::September/20});
}

TEST_F(TiingoATR, RetrievePreviousDataThenComputeAverageTrueRange)
{
    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, 15);

//    std::cout << "\nhistory:\n" << history << '\n';
    EXPECT_EQ(history.size(), 15);
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0]["date"].asString()), date::year_month_day{2021_y/date::October/7});
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[4]["date"].asString()), date::year_month_day{2021_y/date::October/1});

    auto atr = ComputeATR("AAPL", history, 4);
    std::cout << "ATR: " << atr << '\n';
    ASSERT_TRUE(atr == DprDecimal::DDecQuad{"3.36875"});
}

TEST_F(TiingoATR, ComputeATRThenBoxSizeBasedOn20DataPoints)
{
    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    constexpr int history_size = 20;
    const auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, history_size + 1);

    auto atr = ComputeATR("AAPL", history, 4);
//    std::cout << "ATR: " << atr << '\n';
    EXPECT_TRUE(atr == DprDecimal::DDecQuad{"3.36875"});

    // recompute for rest of test

    atr = ComputeATR("AAPL", history, history_size);

    // next, I need to compute my average closing price over the interval 
    // but excluding the 'extra' value included for computing the ATR

    DprDecimal::DDecQuad sum = ranges::accumulate(history | ranges::views::take(history_size),
            DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>(),
            [](const Json::Value& e) { return DprDecimal::DDecQuad{e["close"].asString()}; });

    DprDecimal::DDecQuad box_size = atr / sum;

    std::cout << "box size: " << box_size << '\n';
    box_size.Rescale(".01234");
    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", box_size, 2, PF_Column::FractionalBoxes::e_fractional);

    // ticker data retrieved above is in descending order by date, so let's read it backwards
    // but, there are no reverse iterator provided so let's see if ranges will come to the rescue 
    
//    auto backwards = history | ranges::views::reverse;

    ranges::for_each(history | ranges::views::take(history_size) | ranges::views::reverse, [&chart](const auto& e)
        {
            DprDecimal::DDecQuad val{e["close"].asString()};
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            chart.AddValue(val, date::sys_days(the_date));
        });

    std::cout << chart << '\n';
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
    Tiingo quotes{"api.tiingo.com", "443", "/iex", api_key, "spy,uso,rsp"};
    quotes.Connect();
    bool time_to_stop = false;
    auto the_task = std::async(std::launch::async, &Tiingo::StreamData, &quotes, &time_to_stop);
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


//===  FUNCTION  ======================================================================
//        Name:  InitLogging
// Description:  
//=====================================================================================

void InitLogging ()
{
//    spdlog::set_level(spdlog::level::debug);
//    spdlog::get()->set_filter
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
