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



#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
//#include <algorithm>
//#include <cstdint>
//#include <iostream>
//#include <memory>
//#include <numeric>
//#include <system_error>

#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/numeric/accumulate.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/generate_n.hpp>
#include <range/v3/view/partial_sum.hpp>
#include <range/v3/view/sliding.hpp>
#include <range/v3/view/reverse.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/view/zip_with.hpp>

#include <range/v3/algorithm/equal.hpp>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/drop.hpp>
/* #include <gmock/gmock.h> */
#include <gtest/gtest.h>

#include <date/date.h>
#include <spdlog/spdlog.h>
#include <fmt/format.h>

#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;
using namespace py::literals;

using namespace std::literals::chrono_literals;
using namespace date::literals;
using namespace std::string_literals;
namespace fs = std::filesystem;


// for boost websocket testing

using namespace testing;

#include "DDecQuad.h"
#include "Boxes.h"

#include "PF_Column.h"
#include "PF_Chart.h"
#include "Tiingo.h"

#include "utilities.h"

using namespace DprDecimal;

// some specific files for Testing.

// some utility code for generating test data

std::string MakeSimpleTestData(const std::string& data, const date::year_month_day& first_day, char delim)
{
    auto values = rng_split_string<std::string_view>(data, delim);

    auto holidays = MakeHolidayList(first_day.year());
    ranges::for_each(MakeHolidayList(++(first_day.year())), [&holidays](const auto& e) { holidays.push_back(e); });
    const auto dates = ConstructeBusinessDayList(first_day, ranges::distance(values), UpOrDown::e_Up, &holidays); 
//    auto sample = dates | ranges::views::take(50);
//    std::cout << sample << '\n';

    auto make_test_data = ranges::views::zip_with([](const date::year_month_day& a_date, std::string_view a_value)
            { std::ostringstream test_data; test_data << a_date << ',' << a_value << '\n'; return test_data.str(); }, dates, values);

//    auto sample2 = make_test_data | ranges::views::take(30);
//    std::cout << sample2 << '\n';

    std::string test_data;

    ranges::for_each(make_test_data, [&test_data](const std::string& new_data){ test_data += new_data; } );
    return test_data;
}

std::string MakeSimpleTestData(const std::vector<int32_t>& data, const date::year_month_day& first_day)
{
    // make some business days (although, not doing holidays)
    auto holidays = MakeHolidayList(first_day.year());
    ranges::for_each(MakeHolidayList(++(first_day.year())), [&holidays](const auto& e) { holidays.push_back(e); });
    const auto dates = ConstructeBusinessDayList(first_day, ranges::distance(data), UpOrDown::e_Up, &holidays); 

//    auto sample = dates | ranges::views::take(50);
//    std::cout << sample << '\n';

    auto make_test_data = ranges::views::zip_with([](const date::year_month_day& a_date, int32_t a_value)
            { std::ostringstream test_data; test_data << a_date << ',' << a_value << '\n'; return test_data.str(); }, dates, data);


//    auto sample2 = make_test_data | ranges::views::take(30);
//    std::cout << sample2 << '\n';

    std::string test_data;

    ranges::for_each(make_test_data, [&test_data](const std::string& new_data){ test_data += new_data; } );
    return test_data;
}

std::optional<int> FindColumnIndex (std::string_view header, std::string_view column_name, char delim)
{
    auto fields = rng_split_string<std::string_view>(header, delim);
    auto do_compare([&column_name](const auto& field_name)
    {
        // need case insensitive compare
        // found this on StackOverflow (but modified for my use)
        // (https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c)

        if (column_name.size() != field_name.size())
        {
            return false;
        }
        return ranges::equal(column_name, field_name, [](unsigned char a, unsigned char b) { return tolower(a) == tolower(b); });
    });

    if (auto found_it = ranges::find_if(fields, do_compare); found_it != ranges::end(fields))
    {
        return ranges::distance(ranges::begin(fields), found_it);
    }
    return {};

}		// -----  end of method PF_CollectDataApp::FindColumnIndex  ----- 

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

TEST_F(BusinessDateRange, GenerateMarketHolidays1)
{
    date::year which_year = 2022_y;

    auto holidays = MakeHolidayList(which_year);

    EXPECT_EQ(holidays.size(), 9);      // no new years market holiday for 2022

    for (const auto& [name, date] : holidays)
    {
        std::cout << name << '\t' << date << '\n';
    }

    which_year = 2021_y;

    holidays = MakeHolidayList(which_year);

    EXPECT_EQ(holidays.size(), 9);      // no new years market holiday for 2022

    for (const auto& [name, date] : holidays)
    {
        std::cout << name << '\t' << date << '\n';
    }
}

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

TEST_F(BusinessDateRange, WithinSingleWeekWithHoliday)
{
    auto holidays = MakeHolidayList(2022_y);
    date::year_month_day start_here{2022_y/date::January/16};

    auto result = ConstructeBusinessDayRange(start_here, 4, UpOrDown::e_Up, &holidays);

    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, date::year_month_day{2022_y/date::January/18});
    ASSERT_EQ(result.second, date::year_month_day{2022_y/date::January/21});
}

TEST_F(BusinessDateRange, SpanAWeekWithHoliday)
{
    auto holidays = MakeHolidayList(2022_y);
    date::year_month_day start_here{2022_y/date::January/19};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Down, &holidays);

    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, date::year_month_day{2022_y/date::January/19});
    ASSERT_EQ(result.second, date::year_month_day{2022_y/date::January/12});
}

TEST_F(BusinessDateRange, SpanAWeekAndAMonthAndYearWithHolidays)
{
    // this test spans prior to Thanksgiving till after MLK day
    // but Christmas and New Years are not observed because they 
    // fall on weekends

    auto holidays = MakeHolidayList(2021_y);
    ranges::for_each(MakeHolidayList(2022_y), [&holidays](const auto& e) { holidays.push_back(e); });

    date::year_month_day start_here{2021_y/date::November/22};

    auto result = ConstructeBusinessDayRange(start_here, 43, UpOrDown::e_Up, &holidays);

    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, date::year_month_day{2021_y/date::November/22});
    ASSERT_EQ(result.second, date::year_month_day{2022_y/date::January/24});
}

class DecimalBasicFunctionality : public Test
{

};

TEST_F(DecimalBasicFunctionality, Constructors)
{

    DDecQuad x1;
    DDecQuad x2{"5"};
    DDecQuad x3{"1234.3"s};

    DDecQuad x4{1.25678};

    DDecQuad x5{1.257};

    DDecQuad x6{5.0};
    DDecQuad x7{std::string{"5.0"}};

    EXPECT_EQ(x2, 5);
    EXPECT_EQ(x3, 1234.3);
    EXPECT_NE(x4, 1.257);
    EXPECT_NE(x4, x5);
    
    // test that this works
    EXPECT_EQ(x2, x6);
    EXPECT_EQ(x7, 5);
}

TEST_F(DecimalBasicFunctionality, SimpleArithmetic)
{
    DDecQuad x1{5};
    auto x1_result = x1 + 5;
    EXPECT_EQ(x1_result, 10);

    DDecQuad x2{1.23457};
    auto x2_result = x2 * 2;
    EXPECT_EQ(x2_result, 2.46914);
    EXPECT_TRUE(x2_result == 2.46914);
}

class BoxesBasicFunctionality : public Test
{

};

TEST_F(BoxesBasicFunctionality, Constructors)
{
    Boxes boxes;
    EXPECT_TRUE(boxes.GetBoxsize() == -1);

    Boxes boxes2{DprDecimal::DDecQuad{10}};
    EXPECT_TRUE(boxes2.GetBoxsize() == 10);
}

TEST_F(BoxesBasicFunctionality, GenerateLinearBoxes)
{
    Boxes boxes{DprDecimal::DDecQuad{10}};
    Boxes::Box box = boxes.FindBox(101);
    auto howmany = boxes.GetBoxList().size();
    EXPECT_EQ(howmany, 1);

    // default integral, linear rounds down to nearest integer

    EXPECT_TRUE(box == 100);
    box = boxes.FindBox(109);
    EXPECT_EQ(box, 100);
    howmany = boxes.GetBoxList().size();
    EXPECT_EQ(howmany, 2);

    box = boxes.FindBox(400);
    EXPECT_EQ(box, 400);
    howmany = boxes.GetBoxList().size();

    box = boxes.FindBox(401);
    EXPECT_EQ(box, 400);
    auto howmany2 = boxes.GetBoxList().size();
    EXPECT_NE(howmany, howmany2);

    Boxes boxes2{10, Boxes::BoxType::e_fractional};
    box = boxes2.FindBox(95.5);
    EXPECT_EQ(box, 90);

    // test going smaller

    Boxes boxes3{DprDecimal::DDecQuad{10}};
    box = boxes3.FindBox(100);
    EXPECT_EQ(box, 100);

    box = boxes.FindBox(99);
    EXPECT_EQ(box, 90);

}

TEST_F(BoxesBasicFunctionality, LinearBoxesNextandPrev)
{
    Boxes boxes{DprDecimal::DDecQuad{10}};
    Boxes::Box box = boxes.FindBox(100);
    EXPECT_EQ(box, 100);

    box = boxes.FindNextBox(100);
    EXPECT_EQ(box, 110);

    box = boxes.FindNextBox(100);
    EXPECT_EQ(box, 110);

    Boxes boxes2{DprDecimal::DDecQuad{10}};
    box = boxes2.FindBox(100);
    box = boxes2.FindBox(115);
    EXPECT_EQ(box, 110);

    box = boxes2.FindPrevBox(110);
    EXPECT_EQ(box, 100);

    box = boxes2.FindPrevBox(100);
    EXPECT_EQ(box, 90);
}

TEST_F(BoxesBasicFunctionality, GeneratePercentBoxes)
{
    // I'm not sure how du Plessis is doing his rounding (p. 492) but 
    // I'm using round half up to 3 decimals.

    Boxes boxes{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};

    Boxes::Box box = boxes.FindBox(500);
    EXPECT_EQ(box, 500.00);

    box = boxes.FindBox(506);
    EXPECT_EQ(box, 505.00);

    box = boxes.FindBox(511);
    EXPECT_EQ(box, 510.050);

    box = boxes.FindBox(516);
    EXPECT_EQ(box, 515.151);

    box = boxes.FindBox(532);
    EXPECT_EQ(box, 530.761);

    box = boxes.FindBox(520);
    EXPECT_EQ(box, 515.151);

    // test going smaller

    Boxes boxes2{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};

    box = boxes2.FindBox(505);
    EXPECT_EQ(box, 505);

    box = boxes2.FindBox(500);
    EXPECT_EQ(box, 499.95);

}

TEST_F(BoxesBasicFunctionality, PercentBoxesNextandPrev)
{
    Boxes boxes{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};
    Boxes::Box box = boxes.FindBox(500);
    EXPECT_EQ(box, 500.00);

    box = boxes.FindNextBox(500);
    EXPECT_EQ(box, 505);

    Boxes boxes2{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};
    box = boxes2.FindBox(500);
    box = boxes2.FindBox(505);
    EXPECT_EQ(box, 505);

    box = boxes2.FindPrevBox(505);
    EXPECT_EQ(box, 500);
}

TEST_F(BoxesBasicFunctionality, BoxesToAndFromJson)
{
    const std::string data = "500.0 505.0 510.05 515.151 520.303 525.506 530.761";

    auto values = rng_split_string<std::string>(data, ' ');

    auto prices = values | ranges::views::transform([](const auto& a_value){ DDecQuad result{a_value};  return result; }) | ranges::to<std::vector>();

    Boxes boxes{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};

    ranges::for_each(prices, [&boxes](const auto& x) { boxes.FindBox(x); });

    const auto json = boxes.ToJSON();

    Boxes boxes2{json};

    ASSERT_EQ(boxes, boxes2);
}

TEST_F(BoxesBasicFunctionality, BoxesToJsonThenFromJsonThenAddData)
{
    // first, construct a Boxes using complete set of data

    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    Boxes boxes{DprDecimal::DDecQuad{10}};

    ranges::for_each(prices, [&boxes](const auto& x) { boxes.FindBox(x); });

    // next, construct using part of the data, save to JSON, then load from JSON and add the remaining data 

    const std::vector<int32_t> prices_1 = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129}; 
    const std::vector<int32_t> prices_2 = { 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    Boxes boxes_1{DprDecimal::DDecQuad{10}};
    ranges::for_each(prices_1, [&boxes_1](const auto& x) { boxes_1.FindBox(x); });
    const auto json_1 = boxes_1.ToJSON();

    Boxes boxes_2{json_1};
    ranges::for_each(prices_2, [&boxes_2](const auto& x) { boxes_2.FindBox(x); });

    ASSERT_EQ(boxes, boxes_2);
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

   Boxes boxes{DprDecimal::DDecQuad{5}};
   PF_Column col3{&boxes, 5};
   ASSERT_NE(col3, col2);

}

TEST_F(ColumnFunctionality10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};
    
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
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

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
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    PF_Column::Status status;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    auto json = col.ToJSON();
//    std::cout << json << '\n';

//    EXPECT_EQ(json["box_size"], "10");

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

TEST_F(ColumnFunctionality10X1, ColumnToJsonThenFromJsonThenAddData)
{
    // first, construct a Boxes using complete set of data

    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    PF_Column::Status status;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    // next, construct using part of the data, save to JSON, then load from JSON and add the remaining data 

    const std::vector<int32_t> prices_1 = {1100, 1105, 1110, 1112}; 
    const std::vector<int32_t> prices_2 = {1118, 1120, 1136, 1121, 1129, 1120}; 

    Boxes boxes_1{DprDecimal::DDecQuad{10}};
    PF_Column col_1{&boxes_1, 1};
    ranges::for_each(prices_1, [&col_1, &status, &the_time](auto price) { status = col_1.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    const auto json_1 = boxes_1.ToJSON();
    const auto col_1_json = col_1.ToJSON();

    Boxes boxes_2{json_1};
    PF_Column col_2 = {&boxes_2, col_1_json};
    ranges::for_each(prices_2, [&col_2, &status, &the_time](auto price) { status = col_2.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

//    std::cout << "\n\n col:\n" << col;
//    std::cout << "\n\n col_1:\n" << col_1;
//    std::cout << "\n\n col_2:\n" << col_2 << '\n';;

    EXPECT_EQ(boxes, boxes_2);
    ASSERT_EQ(col, col_2);
}

TEST_F(ColumnFunctionality10X1, ConstructValueStoreAsJSONThenConstructCopy)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    PF_Column::Status status;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    ranges::for_each(prices, [&col, &status, &the_time](auto price) { status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first; });

    auto json = col.ToJSON();
//    std::cout << json << '\n';

    PF_Column col2{&boxes, json};
    EXPECT_EQ(col, col2);
    EXPECT_EQ(col.GetTimeSpan(), col2.GetTimeSpan());
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversal)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    std::vector<PF_Column> columns;

    PF_Column::tpt the_time = std::chrono::system_clock::now();
    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
//        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
//            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
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
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
//        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
//            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
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
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
//        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
//            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1120);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 4);

    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
//    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstHalfOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
//        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
//            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1110);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 5);

    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 1};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(DprDecimal::DDecQuad(price), the_time);
//        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(DprDecimal::DDecQuad(price), the_time).first;
//            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(col.GetTop(), 1140);
    EXPECT_EQ(col.GetBottom(), 1130);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 9);

    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    std::cout << col << '\n';
}

class ColumnFunctionalityFractionalBoxes10X1 : public Test
{

};

TEST_F(ColumnFunctionalityFractionalBoxes10X1, Constructors)
{
    Boxes boxes{DprDecimal::DDecQuad{10}, Boxes::BoxType::e_fractional};
    PF_Column col{&boxes, 1};

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionalityFractionalBoxes10X1, InitialColumnConstructionInitialValueAndDirection)
{
    const std::vector<double> prices = {1100.4, 1105.9, 1110.3, 1112.2, 1118.7, 1120.6}; 
    Boxes boxes{DprDecimal::DDecQuad{10}, Boxes::BoxType::e_fractional};
    PF_Column col{&boxes, 1};
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
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 3};
    
    auto a_value = prices.begin();

    PF_Column::tpt the_time = std::chrono::system_clock::now();
//    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(DprDecimal::DDecQuad{*a_value}, the_time);
//    std::cout << "first value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
//    std::cout << "second value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
//    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(DprDecimal::DDecQuad{*(++a_value)}, the_time);
//    std::cout << "third value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::system_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(DprDecimal::DDecQuad(*a_value), the_time);
//        std::cout << "next value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  " bottom: " << col.GetBottom() << '\n';
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X3, ProcessFirstHalfOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};

    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 3};

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

//    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
//    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X3, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 3};

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

//    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
//    std::cout << col << '\n';
}

class ColumnFunctionality10X5 : public Test
{

};

TEST_F(ColumnFunctionality10X5, Constructors)
{
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 5};

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X5, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 5};

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

//    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
//    std::cout << col << '\n';
}

class ColumnFunctionality10X2 : public Test
{

};

TEST_F(ColumnFunctionality10X2, Constructors)
{
    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 2};

   ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_unknown);

}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestData)
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    Boxes boxes{DprDecimal::DDecQuad{10}};
    PF_Column col{&boxes, 2};

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

//    ranges::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
//    std::cout << col << '\n';
}

class ColumnFunctionalityPercentX1 : public Test
{

};

TEST_F(ColumnFunctionalityPercentX1, SimpleAscendingData)
{
    const std::string data = "500.0 505.0 510.05 515.151 520.303 525.506 530.761";

    // compute a 'simple' ATR by taking successive differences and using that as the true range then compute the ATR using those values.

    auto values = rng_split_string<std::string>(data, ' ');

    auto prices = values | ranges::views::transform([](const auto& a_value){ DDecQuad result{a_value};  return result; }) | ranges::to<std::vector>();
//    ranges::for_each(values, [](const auto& x) { std::cout << x << "  "; });
//    std::cout << '\n';

    const auto value_differences = prices | ranges::views::sliding(2) | ranges::views::transform([](const auto x) { return (x[1] - x[0]).abs(); });
//    ranges::for_each(value_differences, [](const auto& x) { std::cout << x << "  "; });
//    std::cout << '\n';

//    DDecQuad simpleATR = ranges::accumulate(value_differences, DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>()) / static_cast<uint32_t>(value_differences.size());
//    DDecQuad average_value = ranges::accumulate(prices, DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>()) / static_cast<uint32_t>(prices.size());

//    DDecQuad box_size = simpleATR / average_value;
//    DDecQuad box_size = "0.01";
//    box_size.Rescale(-5);
//    std::cout << "box_size: " << box_size << '\n';
//    EXPECT_EQ(box_size, 0.01);

    Boxes boxes{0.01, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent};
    PF_Column col{&boxes, 2};

    std::vector<PF_Column> columns;
    PF_Column::tpt the_time = std::chrono::system_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(price, the_time);
//        std::cout << "value: " << price << " status: " << status << " top: " << col.GetTop() << '\n';
        if (status == PF_Column::Status::e_reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.
            
            status = col.AddValue(price, the_time).first;
        }
    }

    std::cout << "Column: " << col << '\n';

    EXPECT_EQ(col.GetTop(), 530.761);
    EXPECT_EQ(col.GetBottom(), 500.00);

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

    std::string test_data = MakeSimpleTestData(data, date::year_month_day {2015_y/date::March/date::Monday[1]}, ' ');

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

TEST_F(ChartFunctionality10X2, ProcessSomeDataThenToJSONThenFromJSONThenMoreData)
{
    const std::vector<int32_t> values_ints = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    std::string test_data = MakeSimpleTestData(values_ints, date::year_month_day {2015_y/date::March/date::Monday[1]});

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    const std::vector<int32_t> values_ints_1 = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};
    const std::vector<int32_t> values_ints_2 = {1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 
    std::string test_data_1 = MakeSimpleTestData(values_ints_1, date::year_month_day {2015_y/date::March/date::Monday[1]});
    std::string test_data_2 = MakeSimpleTestData(values_ints_2, date::year_month_day {2015_y/date::March/date::Monday[1]});

    std::istringstream prices_1{test_data_1}; 

    PF_Chart chart_1("GOOG", 10, 2);
    chart_1.LoadData(&prices_1, "%Y-%m-%d", ',');

    const auto chart_1_json = chart_1.ToJSON();

    PF_Chart chart_2{chart_1_json};
    std::istringstream prices_2{test_data_2}; 
    chart_2.LoadData(&prices_2, "%Y-%m-%d", ',');

//    std::cout << "\n\n chart:\n" << chart;
//    std::cout << "\n\n chart_1:\n" << chart_1;
//    std::cout << "\n\n chart_2:\n" << chart_2 << '\n';;

    ASSERT_EQ(chart, chart_2);
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsInts)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2, Boxes::BoxType::e_fractional);
//    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(chart.GetNumberOfColumns(), 47);

    EXPECT_EQ(chart[47].GetTop(), 148);
    EXPECT_EQ(chart[47].GetBottom(), 146);

//    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsThenJSON)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2, Boxes::BoxType::e_fractional);
//    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    auto json = chart.ToJSON();

    EXPECT_EQ(json["current_direction"].asString(), "up");
    EXPECT_EQ(json["columns"].size(), 46);

    EXPECT_EQ(json["columns"][45]["top"].asString(), "146");
    EXPECT_EQ(json["columns"][45]["bottom"].asString(), "144");

//    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsToJSONFromJSON)
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2, Boxes::BoxType::e_fractional);
//    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    auto json = chart.ToJSON();

    PF_Chart chart2{json};
    ASSERT_EQ(chart, chart2);

//    std::cout << chart << '\n';
}

// use ATR computed box size instead of predefined box size 

class ChartFunctionalitySimpleATRX2 : public Test
{

};

TEST_F(ChartFunctionalitySimpleATRX2, ComputeATRBoxSizeForFirstSetOfTestData)
{
    const std::vector<int32_t> values_ints = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    const auto value_differences = values_ints | ranges::views::sliding(2) | ranges::views::transform([](const auto x) { return abs(x[1] - x[0]); });
//    ranges::for_each(value_differences, [](const auto& x) { std::cout << x << "  "; });
//    std::cout << '\n';

    EXPECT_EQ(value_differences[0], 5);
    EXPECT_EQ(value_differences[6], 15);
    EXPECT_EQ(value_differences[value_differences.size() -1], 9);

    DDecQuad simpleATR = static_cast<double>(ranges::accumulate(value_differences, 0)) / static_cast<double>(value_differences.size());
    DDecQuad average_value = static_cast<double>(ranges::accumulate(values_ints, 0)) / static_cast<double>(values_ints.size());
    DDecQuad box_size = simpleATR / average_value;
//    box_size.Rescale(".01234");
    box_size.Rescale(-5);
    std::cout << "box_size: " << box_size << '\n';

    EXPECT_EQ(box_size, 0.01150);
//    EXPECT_EQ(chart[5].GetBottom(), 1130);
//    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

TEST_F(ChartFunctionalitySimpleATRX2, ProcessCompletelyFirstSetOfTestDataWithATR)
{
    const std::vector<int32_t> values_ints = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    const auto value_differences = values_ints | ranges::views::sliding(2) | ranges::views::transform([](const auto x) { return abs(x[1] - x[0]); });
//    ranges::for_each(value_differences, [](const auto& x) { std::cout << x << "  "; });
//    std::cout << '\n';

    DDecQuad simpleATR = static_cast<double>(ranges::accumulate(value_differences, 0)) / static_cast<double>(value_differences.size());
    DDecQuad average_value = static_cast<double>(ranges::accumulate(values_ints, 0)) / static_cast<double>(values_ints.size());
    DDecQuad box_size = simpleATR / average_value;
//    box_size.Rescale(".01234");
    box_size.Rescale(-5);
    std::cout << "box_size: " << box_size << '\n';

    EXPECT_EQ(box_size, 0.01150);

    std::string test_data = MakeSimpleTestData(values_ints, date::year_month_day {2015_y/date::March/date::Monday[1]});

    std::istringstream prices{test_data}; 

//    double factor = (10.0 / box_size).ToDouble();

    PF_Chart chart("GOOG", box_size, 2, Boxes::BoxType::e_fractional);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

//    std::cout << chart << '\n';

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 36);
//
//    EXPECT_EQ(chart[5].GetTop(), 1140);
//    EXPECT_EQ(chart[5].GetBottom(), 1130);
//    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

class MiscChartFunctionality : public Test
{

};

TEST_F(MiscChartFunctionality, LoadDataFromJSONChartFileThenAddDataFromCSV)
{
    fs::path symbol_file_name{"./test_files/SPY_1.json"};

    const std::string file_content = LoadDataFileForUse(symbol_file_name);

    JSONCPP_STRING err;
    Json::Value saved_data;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (! reader->parse(file_content.data(), file_content.data() + file_content.size(), &saved_data, &err))
    {
        throw std::runtime_error("Problem parsing test data file: "s + err);
    }

    PF_Chart new_chart{saved_data};
    std::cout << new_chart << '\n';
    
    fs::path csv_file_name{"./test_files3/SPY.csv"};
    const std::string file_content_csv = LoadDataFileForUse(csv_file_name);

    const auto symbol_data_records = split_string<std::string_view>(file_content_csv, '\n');
    const auto header_record = symbol_data_records.front();

    auto date_column = FindColumnIndex(header_record, "date", ',');
    BOOST_ASSERT_MSG(date_column.has_value(), fmt::format("Can't find 'date' field in header record: {}.", header_record).c_str());
    
    auto close_column = FindColumnIndex(header_record, "Close", ',');
    BOOST_ASSERT_MSG(close_column.has_value(), fmt::format("Can't find price field: 'Close' in header record: {}.", header_record).c_str());

    std::cout << "new chart at start of adding new data: \n\n" << new_chart << "\n\n";
    ranges::for_each(symbol_data_records | ranges::views::drop(1), [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record)
        {
//            std::cout << "len: " << record.size() << "  " << record << '\n';
            const auto fields = split_string<std::string_view> (record, ',');
//            std::cout << "close value: " << fields[close_col] << " date value: " << fields[date_col] << " record: \n" << record << '\n';
            new_chart.AddValue(DprDecimal::DDecQuad(fields[close_col]), StringToTimePoint("%Y-%m-%d", fields[date_col]));
        });
    std::cout << "new chart at AFTER adding new data: \n\n" << new_chart << "\n\n";
}

// use ATR computed box size instead of predefined box size with logarithmic charts 

//class ColumnFunctionalityLogX1 : public Test
//{
//
//};
//
class PercentChartFunctionalitySimpleATRX2 : public Test
{

};

TEST_F(PercentChartFunctionalitySimpleATRX2, ProcessCompletelyFirstSetOfTestDataWithATR)
{
    const std::vector<int32_t> values_ints = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129,
        1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127}; 

    // compute a 'simple' ATR by taking successive differences and using that as the true range then compute the ATR using those values.

    const auto value_differences = values_ints | ranges::views::sliding(2) | ranges::views::transform([](const auto x) { return abs(x[1] - x[0]); });

    DDecQuad simpleATR = double{static_cast<double>(ranges::accumulate(value_differences, double{0.0}))} / double{static_cast<double>(value_differences.size())};
    DDecQuad average_value = double{static_cast<double>(ranges::accumulate(values_ints, 0))} / double{static_cast<double>(values_ints.size())};
    DDecQuad box_size = simpleATR / average_value;
    box_size.Rescale(-5);
    std::cout << "box_size: " << box_size << '\n';
    std::string test_data = MakeSimpleTestData(values_ints, date::year_month_day {2015_y/date::March/date::Monday[1]});

    std::istringstream prices{test_data}; 

//    double factor = (10.0 / box_size).ToDouble();

    PF_Chart chart("GOOG", box_size, 2, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

//    std::cout << chart << '\n';

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(chart.GetNumberOfColumns(), 5);
//
//    EXPECT_EQ(chart[5].GetTop(), 1140);
//    EXPECT_EQ(chart[5].GetBottom(), 1130);
//    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

class PlotChartsWithChartDirector : public Test
{

};

TEST_F(PlotChartsWithChartDirector, Plot10X1Chart)
{
    if (fs::exists("/tmp/candlestick1.svg"))
    {
        fs::remove("/tmp/candlestick1.svg");
    }
    const std::string data = "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 " \
    "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::string test_data = MakeSimpleTestData(data, date::year_month_day {2015_y/date::March/date::Monday[1]}, ' ');

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 1);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

//    std::cout << chart << '\n';

    chart.ConstructChartGraphAndWriteToFile("/tmp/candlestick1.svg");

    ASSERT_TRUE(fs::exists("/tmp/candlestick1.svg"));
}

TEST_F(PlotChartsWithChartDirector, Plot10X2Chart)
{
    if (fs::exists("/tmp/candlestick.svg"))
    {
        fs::remove("/tmp/candlestick.svg");
    }
    const std::string data = "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 1108 1092 1129 " \
    "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 1159 1136 1127";

    std::string test_data = MakeSimpleTestData(data, date::year_month_day {2015_y/date::March/date::Monday[1]}, ' ');

    std::istringstream prices{test_data}; 

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ',');

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    EXPECT_EQ(chart.GetNumberOfColumns(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

//    std::cout << chart << '\n';

    chart.ConstructChartGraphAndWriteToFile("/tmp/candlestick.svg");

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

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_up);
    EXPECT_EQ(chart.GetNumberOfColumns(), 47);

    EXPECT_EQ(chart[47].GetTop(), 148);
    EXPECT_EQ(chart[47].GetBottom(), 146);

//    std::cout << chart << '\n';

    chart.ConstructChartGraphAndWriteToFile("/tmp/candlestick2.svg");
    
    ASSERT_TRUE(fs::exists("/tmp/candlestick2.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalDataUsingComputedATR)
{
    if (fs::exists("/tmp/candlestick3.svg"))
    {
        fs::remove("/tmp/candlestick3.svg");
    }
    const fs::path file_name{"./test_files/APPLE.json"};

    const std::string hist = LoadDataFileForUse("./test_files/APPLE.json");
    std::cout << "history length: " << hist.size() << '\n';

    const std::regex source{R"***("(open|high|low|close|adjOpen|adjHigh|adjLow|adjClose)":([0-9]*\.[0-9]*))***"}; 
    const std::string dest{R"***("$1":"$2")***"};
    auto result1 = std::regex_replace(hist, source, dest);
    std::cout << "result length: " << result1.size() << '\n';
//    std::cout.write(result1.data(), 900);
//    std::cout << " <== data\n";

    JSONCPP_STRING err;
    Json::Value history;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (! reader->parse(result1.data(), result1.data() + result1.size(), &history, &err))
    {
        throw std::runtime_error("Problem parsing test data file: "s + err);
    }
    std::cout << "history length: " << history.size() << '\n';

    auto atr = ComputeATR("AAPL", history, history.size() -1, UseAdjusted::e_Yes);

    std::cout << "ATR: " << atr << '\n';

//    // next, I need to compute my average closing price over the interval 
//    // but excluding the 'extra' value included for computing the ATR
//
//    // I'm doing this crazy const casting because the range wants only a const input source
//    // and mine isn't.  Used below also.
//
//    DprDecimal::DDecQuad sum = ranges::accumulate(*const_cast<const Json::Value*>(&history) | ranges::views::reverse | ranges::views::take(history.size() - 1),
//            DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>(),
//            [](const Json::Value& e) { return DprDecimal::DDecQuad{e["adjClose"].asString()}; });

    // compute box size as a percent of ATR, eg. 0.1

//    DprDecimal::DDecQuad box_size = atr / (sum / (history.size() - 1));
    DprDecimal::DDecQuad box_size = atr * 0.1;

    std::cout << "box size: " << box_size << '\n';
    box_size.Rescale(-5);
    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", box_size, 3, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_linear);

//    ranges::for_each(*const_cast<const Json::Value*>(&history) | ranges::views::reverse | ranges::views::take(50), [&chart](const auto& e)
    ranges::for_each(*const_cast<const Json::Value*>(&history) | ranges::views::reverse | ranges::views::take(history.size() - 1), [&chart](const auto& e)
        {
//            std::cout << "processing: " << e << '\n';
            DprDecimal::DDecQuad val{e["adjClose"].asString()};
//            std::cout << "val: " << val << '\n';
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            chart.AddValue(val, date::sys_days(the_date));
        });

//    PF_Chart chart("AAPL", 2, 2);
//    chart.LoadData(&prices, "%Y-%m-%d", ',');

//    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
//    EXPECT_EQ(chart.GetNumberOfColumns(), 62);
//
//    EXPECT_EQ(chart[61].GetTop(), 146);
//    EXPECT_EQ(chart[61].GetBottom(), 144);

//    std::cout << chart << '\n';
//
    chart.ConstructChartGraphAndWriteToFile("/tmp/candlestick3.svg");
    
    ASSERT_TRUE(fs::exists("/tmp/candlestick3.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalDataUsingBothArithmeticAndPercent)
{
    if (fs::exists("/tmp/candlestick3.svg"))
    {
        fs::remove("/tmp/candlestick3.svg");
    }
    if (fs::exists("/tmp/candlestick4.svg"))
    {
        fs::remove("/tmp/candlestick4.svg");
    }

    const std::string hist = LoadDataFileForUse("./test_files/YAHOO.json");
    std::cout << "history length: " << hist.size() << '\n';

    const std::regex source{R"***("(open|high|low|close|adjOpen|adjHigh|adjLow|adjClose)":([0-9]*\.[0-9]*))***"}; 
    const std::string dest{R"***("$1":"$2")***"};
    auto result1 = std::regex_replace(hist, source, dest);

    JSONCPP_STRING err;
    Json::Value history;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (! reader->parse(result1.data(), result1.data() + result1.size(), &history, &err))
    {
        throw std::runtime_error("Problem parsing test data file: "s + err);
    }
    std::cout << "history length: " << history.size() << '\n';

    DDecQuad box_size = .01;

    PF_Chart chart("YHOO", box_size, 3, Boxes::BoxType::e_fractional);

    ranges::for_each(*const_cast<const Json::Value*>(&history) | ranges::views::reverse | ranges::views::take(history.size() - 1), [&chart](const auto& e)
        {
            DprDecimal::DDecQuad val{e["adjClose"].asString()};
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            chart.AddValue(val, date::sys_days(the_date));
        });

    std::cout << "# of cols: " << chart.GetNumberOfColumns() << '\n';

    chart.ConstructChartGraphAndWriteToFile("/tmp/candlestick3.svg");
    
    EXPECT_TRUE(fs::exists("/tmp/candlestick3.svg"));

    PF_Chart chart_percent("YHOO", box_size, 3, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent);

    ranges::for_each(*const_cast<const Json::Value*>(&history) | ranges::views::reverse | ranges::views::take(history.size() - 1), [&chart_percent](const auto& e)
        {
            DprDecimal::DDecQuad val{e["adjClose"].asString()};
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            chart_percent.AddValue(val, date::sys_days(the_date));
        });

//    std::cout << chart_percent << '\n';
    std::cout << "# of cols: " << chart_percent.GetNumberOfColumns() << '\n';

    chart_percent.ConstructChartGraphAndWriteToFile("/tmp/candlestick4.svg");
    
    EXPECT_TRUE(fs::exists("/tmp/candlestick4.svg"));
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
    date::year which_year = 2021_y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, 14, &holidays);

    EXPECT_EQ(history.size(), 14);
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0]["date"].asString()), date::year_month_day{2021_y/date::October/7});
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[13]["date"].asString()), date::year_month_day{2021_y/date::September/20});
}

TEST_F(TiingoATR, RetrievePreviousDataThenComputeAverageTrueRange)
{
    date::year which_year = 2021_y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, 15, &holidays);

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
    date::year which_year = 2021_y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    constexpr int history_size = 20;
    const auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, history_size + 1, &holidays);

    auto atr = ComputeATR("AAPL", history, 4);
//    std::cout << "ATR: " << atr << '\n';
    EXPECT_TRUE(atr == DprDecimal::DDecQuad{"3.36875"});

    // recompute using all the data for rest of test

    atr = ComputeATR("AAPL", history, history_size);

    // next, I need to compute my average closing price over the interval 
    // but excluding the 'extra' value included for computing the ATR

    DprDecimal::DDecQuad sum = ranges::accumulate(history | ranges::views::reverse | ranges::views::take(history_size),
            DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>(),
            [](const Json::Value& e) { return DprDecimal::DDecQuad{e["close"].asString()}; });

    DprDecimal::DDecQuad box_size = atr / (sum / history_size);

    std::cout << "box size: " << box_size << '\n';
    box_size.Rescale(-5);
    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", box_size, 2, Boxes::BoxType::e_fractional);

    // ticker data retrieved above is in descending order by date, so let's read it backwards
    // but, there are no reverse iterator provided so let's see if ranges will come to the rescue 
    
//    auto backwards = history | ranges::views::reverse;

    ranges::for_each(history | ranges::views::reverse | ranges::views::take(history_size), [&chart](const auto& e)
        {
            DprDecimal::DDecQuad val{e["close"].asString()};
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            chart.AddValue(val, date::sys_days(the_date));
        });

//    std::cout << chart << '\n';
}

TEST_F(TiingoATR, ComputeATRThenBoxSizeBasedOn20DataPointsUsePercentValues)
{
    date::year which_year = 2021_y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key};

    constexpr int history_size = 20;
    const auto history = history_getter.GetMostRecentTickerData("AAPL", date::year_month_day{2021_y/date::October/7}, history_size + 1, &holidays);

    auto atr = ComputeATR("AAPL", history, 4, UseAdjusted::e_Yes);
//    std::cout << "ATR: " << atr << '\n';
    EXPECT_EQ(atr, DprDecimal::DDecQuad{"3.36875"});

    // recompute using all the data for rest of test

    atr = ComputeATR("AAPL", history, history_size);

    // next, I need to compute my average closing price over the interval 
    // but excluding the 'extra' value included for computing the ATR

    DprDecimal::DDecQuad sum = ranges::accumulate(history | ranges::views::reverse | ranges::views::take(history_size),
            DprDecimal::DDecQuad{}, std::plus<DprDecimal::DDecQuad>(),
            [](const Json::Value& e) { return DprDecimal::DDecQuad{e["adjClose"].asString()}; });

    DprDecimal::DDecQuad box_size = atr / (sum / history_size);

    std::cout << "box size: " << box_size << '\n';
    box_size.Rescale(-5);
    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", box_size, 2, Boxes::BoxType::e_fractional, Boxes::BoxScale::e_percent);

    // ticker data retrieved above is in descending order by date, so let's read it backwards
    // but, there are no reverse iterator provided so let's see if ranges will come to the rescue 
    
//    auto backwards = history | ranges::views::reverse;

    ranges::for_each(history | ranges::views::reverse | ranges::views::take(history_size), [&chart](const auto& e)
        {
            DprDecimal::DDecQuad val{e["adjClose"].asString()};
            std::string dte{e["date"].asString()};
            std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
            date::year_month_day the_date = StringToDateYMD("%Y-%m-%d", date);
            auto status = chart.AddValue(val, date::sys_days(the_date));
            std::cout << "value: " << val << " status: " << status << '\n';
        });

//    std::cout << chart << '\n';

//    ranges::for_each(history | ranges::views::reverse , [](const auto& e) { std::cout << fmt::format("date: {} close: {} adjusted close: {} delta: {} \n",
//                e["date"].asString(), e["close"].asString(), e["adjClose"].asString(), 0); });
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
    Tiingo quotes{"api.tiingo.com", "443", "/iex", api_key, std::vector<std::string> {"spy","uso","rsp"}};
    quotes.Connect();
    bool time_to_stop = false;
    auto the_task = std::async(std::launch::async, &Tiingo::StreamDataTest, &quotes, &time_to_stop);
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

    py::scoped_interpreter guard{false}; // start the interpreter and keep it alive

    py::print("Hello, World!"); // use the Python API

    py::exec(R"(
        import pandas as pd 
        import matplotlib.pyplot as plt
        import mplfinance as mpf
        )"
    );
    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
