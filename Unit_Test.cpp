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

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <initializer_list>
#include <iterator>
#include <numeric>
#include <ranges>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>

namespace rng = std::ranges;
namespace vws = std::ranges::views;

#include <date/tz.h>
#include <gtest/gtest.h>
// #include <pybind11/embed.h>  // everything needed for embedding
#include <spdlog/spdlog.h>

#include <pqxx/pqxx>
#include <pqxx/transaction.hxx>
#include <range/v3/range/conversion.hpp>
// namespace py = pybind11;
// using namespace py::literals;

#include <decimal.hh>

using decimal::Decimal;

using namespace std::literals::chrono_literals;
using namespace std::string_literals;
namespace fs = std::filesystem;

// for boost websocket testing

using namespace testing;

#include "Boxes.h"
#include "ConstructChartGraphic.h"
#include "PF_Chart.h"
#include "PF_Column.h"
#include "PF_Signals.h"
#include "PointAndFigureDB.h"
#include "Tiingo.h"
#include "utilities.h"

// NOLINTBEGIN(*-magic-numbers)
//

// some specific files for Testing.

// some utility code for generating test data

std::string MakeSimpleTestData(const std::string& data, const std::chrono::year_month_day& first_day,
                               std::string_view delim)
{
    auto values = rng_split_string<std::string_view>(data, delim);

    auto holidays = MakeHolidayList(first_day.year());
    rng::copy(MakeHolidayList(++(first_day.year())), std::back_inserter(holidays));
    const auto dates = ConstructeBusinessDayList(first_day, rng::distance(values), UpOrDown::e_Up, &holidays);
    //    auto sample = dates | vws::take(50);
    //    std::cout << sample << '\n';

    auto test_values = vws::zip(dates, values);

    std::string test_data;

    for (const auto& [date, value] : test_values)
    {
        test_data.append(std::format("{},{}\n", date, value));
    }
    return test_data;
}

std::string MakeSimpleTestData(const std::vector<int32_t>& data, const std::chrono::year_month_day& first_day)
{
    // make some business days (although, not doing holidays)
    auto holidays = MakeHolidayList(first_day.year());
    rng::copy(MakeHolidayList(++(first_day.year())), std::back_inserter(holidays));
    const auto dates = ConstructeBusinessDayList(first_day, rng::size(data), UpOrDown::e_Up, &holidays);

    //    auto sample = dates | vws::take(50);
    //    std::cout << sample << '\n';
    auto test_values = vws::zip(dates, data);

    std::string test_data;

    for (const auto& [date, value] : test_values)
    {
        test_data.append(std::format("{},{}\n", date, value));
    }
    return test_data;
}

std::optional<int> FindColumnIndex(std::string_view header, std::string_view column_name, std::string_view delim)
{
    auto fields = rng_split_string<std::string_view>(header, delim);
    auto do_compare(
        [&column_name](const auto& field_name)
        {
            // need case insensitive compare
            // found this on StackOverflow (but modified for my use)
            // (https://stackoverflow.com/questions/11635/case-insensitive-string-comparison-in-c)

            if (column_name.size() != field_name.size())
            {
                return false;
            }
            return rng::equal(column_name, field_name,
                              [](unsigned char a, unsigned char b) { return tolower(a) == tolower(b); });
        });

    if (auto found_it = rng::find_if(fields, do_compare); found_it != rng::end(fields))
    {
        return rng::distance(rng::begin(fields), found_it);
        // return rng::size(rng::subrange(rng::begin(fields), found_it));
    }
    return {};

}  // -----  end of method PF_CollectDataApp::FindColumnIndex  -----

class RangeSplitterBasicFunctionality : public Test
{
};

TEST_F(RangeSplitterBasicFunctionality, Test1)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    auto values = split_string<std::string_view>(data, " ");

    auto items = rng_split_string<std::string_view>(data, " ");

    EXPECT_EQ(values.size(), rng::distance(items));

    std::vector<std::string_view> values2;

    rng::for_each(items, [&values2](const auto& x) { values2.push_back(x); });

    ASSERT_EQ(values, values2);
};

class Timer : public Test
{
};

TEST_F(Timer, DISABLED_TestCountDownTimer)  // NOLINT
{
    // a 10 second count down

    auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                          floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    auto then = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                           floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 10s);

    int counter = 0;
    auto timer = [&counter](const auto& stop_at)
    {
        while (true)
        {
            std::cout << "ding...\n";
            ++counter;
            auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                  floor<std::chrono::seconds>(std::chrono::system_clock::now()));
            if (now.get_sys_time() >= stop_at.get_sys_time())
            {
                break;
            }
            std::this_thread::sleep_for(1s);
        }
    };

    auto timer_task = std::async(std::launch::async, timer, then);

    std::this_thread::sleep_for(5s);
    timer_task.get();

    EXPECT_TRUE(counter >= 9);
};

class BusinessDateRange : public Test
{
};

TEST_F(BusinessDateRange, GenerateMarketHolidays1)  // NOLINT
{
    std::chrono::year which_year = 2022y;

    auto holidays = MakeHolidayList(which_year);

    EXPECT_EQ(holidays.size(), 9);  // no new years market holiday for 2022

    for (const auto& [name, date] : holidays)
    {
        //        std::cout << name << '\t' << date << '\n';
    }

    which_year = 2021y;

    holidays = MakeHolidayList(which_year);

    EXPECT_EQ(holidays.size(), 9);  // no new years market holiday for 2022

    //     for (const auto& [name, date] : holidays)
    //     {
    // //        std::cout << name << '\t' << date << '\n';
    //     }
}

TEST_F(BusinessDateRange, WithinSingleWeek)  // NOLINT
{
    std::chrono::year_month_day start_here{2021y / std::chrono::October / std::chrono::Friday[1]};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Down);

    //    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, std::chrono::year_month_day{2021y / std::chrono::September / 27});
}

TEST_F(BusinessDateRange, SpanAWeek)  // NOLINT
{
    std::chrono::year_month_day start_here{2021y / std::chrono::October / std::chrono::Friday[1]};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Up);

    //    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, std::chrono::year_month_day{2021y / std::chrono::October / 7});
}

TEST_F(BusinessDateRange, SpanAWeekAndAMonth)  // NOLINT
{
    std::chrono::year_month_day start_here{2021y / std::chrono::September / 22};

    auto result = ConstructeBusinessDayRange(start_here, 12, UpOrDown::e_Up);

    //    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.second, std::chrono::year_month_day{2021y / std::chrono::October / 7});

    start_here = std::chrono::year_month_day{2021y / std::chrono::October / 7};

    result = ConstructeBusinessDayRange(start_here, 14, UpOrDown::e_Down);

    //    std::cout << result.first << " : " << result.second << '\n';

    ASSERT_EQ(result.second, std::chrono::year_month_day{2021y / std::chrono::September / 20});
}

TEST_F(BusinessDateRange, WithinSingleWeekWithHoliday)  // NOLINT
{
    auto holidays = MakeHolidayList(2022y);
    std::chrono::year_month_day start_here{2022y / std::chrono::January / 16};

    auto result = ConstructeBusinessDayRange(start_here, 4, UpOrDown::e_Up, &holidays);

    //    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, std::chrono::year_month_day{2022y / std::chrono::January / 18});
    ASSERT_EQ(result.second, std::chrono::year_month_day{2022y / std::chrono::January / 21});
}

TEST_F(BusinessDateRange, SpanAWeekWithHoliday)  // NOLINT
{
    auto holidays = MakeHolidayList(2022y);
    std::chrono::year_month_day start_here{2022y / std::chrono::January / 19};

    auto result = ConstructeBusinessDayRange(start_here, 5, UpOrDown::e_Down, &holidays);

    //    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, std::chrono::year_month_day{2022y / std::chrono::January / 19});
    ASSERT_EQ(result.second, std::chrono::year_month_day{2022y / std::chrono::January / 12});
}

TEST_F(BusinessDateRange, SpanAWeekAndAMonthAndYearWithHolidays)  // NOLINT
{
    // this test spans prior to Thanksgiving till after MLK day
    // but Christmas and New Years are not observed because they
    // fall on weekends

    auto holidays = MakeHolidayList(2021y);
    rng::copy(MakeHolidayList(2022y), std::back_inserter(holidays));

    std::chrono::year_month_day start_here{2021y / std::chrono::November / 22};

    auto result = ConstructeBusinessDayRange(start_here, 43, UpOrDown::e_Up, &holidays);

    //    std::cout << result.first << " : " << result.second << '\n';

    EXPECT_EQ(result.first, std::chrono::year_month_day{2021y / std::chrono::November / 22});
    ASSERT_EQ(result.second, std::chrono::year_month_day{2022y / std::chrono::January / 24});
}

class CanWeStreamNow : public Test
{
};

TEST_F(CanWeStreamNow, IsTheMarketOpenVariousTimeZones)  // NOLINT
{
    // we construct our test time using local_days because we are specifying the
    // time we want it to be in the given timezone. This is what local_days does.

    // in this test, Perth is 12 hours ahead of New York so it's actually still
    // Sunday in NY.

    const auto time1 = std::chrono::local_days{2022y / std::chrono::March / 14} + 8h + 30min + 0s;
    auto market_status = GetUS_MarketStatus("Australia/Perth", time1);
    //    std::cout << "Perth Australia: " << market_status << '\n';
    EXPECT_EQ(market_status, US_MarketStatus::e_NonTradingDay);

    const auto time2 = std::chrono::local_days{2022y / std::chrono::March / 14} + 8h + 30min + 0s;
    market_status = GetUS_MarketStatus("America/Los_Angeles", time2);
    //    std::cout << "Los Angeles: " << market_status << '\n';
    EXPECT_EQ(market_status, US_MarketStatus::e_OpenForTrading);

    // DST in New York, standard time in London

    const auto time3 = std::chrono::local_days{2022y / std::chrono::March / 14} + 20h + 30min + 0s;
    market_status = GetUS_MarketStatus("Europe/London", time3);
    //    std::cout << "London: " << market_status << '\n';
    EXPECT_EQ(market_status, US_MarketStatus::e_ClosedForDay);

    // DST in New York, DST in London

    const auto time3a = std::chrono::local_days{2022y / std::chrono::March / 29} + 20h + 30min + 0s;
    market_status = GetUS_MarketStatus("Europe/London", time3a);
    //    std::cout << "London: " << market_status << '\n';
    EXPECT_EQ(market_status, US_MarketStatus::e_OpenForTrading);

    const auto time4 = std::chrono::local_days{2022y / std::chrono::March / 14} + 6h + 0min + 0s;
    market_status = GetUS_MarketStatus("America/Chicago", time4);
    //    std::cout << "Chicago: " << market_status << '\n';
    EXPECT_EQ(market_status, US_MarketStatus::e_NotOpenYet);
}

class DecimalBasicFunctionality : public Test
{
};

TEST_F(DecimalBasicFunctionality, Constructors)  // NOLINT
{
    Decimal x1;
    Decimal x2{"5"};
    Decimal x3{"1234.3"s};

    Decimal x4("1.25678");

    Decimal x5("1.257");

    Decimal x6("5.0");
    Decimal x7{std::string{"5.0"}};

    EXPECT_EQ(x2, 5);
    EXPECT_EQ(x3, Decimal("1234.3"));
    EXPECT_NE(x4, Decimal("1.257"));
    EXPECT_NE(x4, x5);

    // test that this works
    EXPECT_EQ(x2, x6);
    EXPECT_EQ(x7, 5);
}

TEST_F(DecimalBasicFunctionality, SimpleArithmetic)  // NOLINT
{
    Decimal x1{5};
    auto x1_result = x1 + 5;
    EXPECT_EQ(x1_result, 10);

    Decimal x2("1.23457");
    auto x2_result = x2 * 2;
    EXPECT_EQ(x2_result, Decimal("2.46914"));
    EXPECT_TRUE(x2_result == Decimal("2.46914"));
}

TEST_F(DecimalBasicFunctionality, SimpleLog_nUsage)  // NOLINT
{
    Decimal x1{"500.5"};
    auto x1_ln = x1.ln();
    auto x1_dec = x1_ln.exp();
    EXPECT_EQ(x1, x1_dec.rescale(x1.exponent()));

    // Decimal x2("1.23457");
    // auto x2_result = x2 * 2;
    // EXPECT_EQ(x2_result, Decimal("2.46914"));
    // EXPECT_TRUE(x2_result == Decimal("2.46914"));
}
class BoxesBasicFunctionality : public Test
{
};

TEST_F(BoxesBasicFunctionality, Constructors)  // NOLINT
{
    Boxes boxes;
    EXPECT_TRUE(boxes.GetBoxSize() == -1);

    Boxes boxes2{Decimal{10}};
    EXPECT_TRUE(boxes2.GetBoxSize() == 10);
}

TEST_F(BoxesBasicFunctionality, GenerateLinearBoxes)  // NOLINT
{
    Boxes boxes{Decimal{10}};
    Boxes::Box box = boxes.FindBox(101);
    auto howmany = boxes.GetBoxList().size();
    EXPECT_EQ(howmany, 1);

    // default integral, linear rounds down to nearest integer

    EXPECT_TRUE(box == 101);
    box = boxes.FindBox(109);
    EXPECT_EQ(box, 101);
    howmany = boxes.GetBoxList().size();
    EXPECT_EQ(howmany, 2);

    box = boxes.FindBox(400);
    EXPECT_EQ(box, 391);
    howmany = boxes.GetBoxList().size();

    box = boxes.FindBox(401);
    EXPECT_EQ(box, 401);
    auto howmany2 = boxes.GetBoxList().size();
    EXPECT_EQ(howmany, howmany2);

    Boxes boxes2{10.0};
    box = boxes2.FindBox(Decimal("95.5"));
    EXPECT_EQ(box, 96);  // box will be integral, so rounds .5 up

    // test going smaller

    Boxes boxes3{Decimal{10}};
    box = boxes3.FindBox(100);
    EXPECT_EQ(box, 100);

    box = boxes.FindBox(99);
    EXPECT_EQ(box, 91);
}

TEST_F(BoxesBasicFunctionality, LinearBoxesNextandPrev)  // NOLINT
{
    Boxes boxes{Decimal{10}};
    Boxes::Box box = boxes.FindBox(100);
    EXPECT_EQ(box, 100);

    box = boxes.FindNextBox(100);
    EXPECT_EQ(box, 110);

    box = boxes.FindNextBox(100);
    EXPECT_EQ(box, 110);

    Boxes boxes2{Decimal{10}};
    box = boxes2.FindBox(100);
    box = boxes2.FindBox(115);
    EXPECT_EQ(box, 110);

    box = boxes2.FindPrevBox(110);
    EXPECT_EQ(box, 100);

    box = boxes2.FindPrevBox(100);
    EXPECT_EQ(box, 90);
}

TEST_F(BoxesBasicFunctionality, GeneratePercentBoxes)  // NOLINT
{
    // I'm not sure how du Plessis is doing his rounding (p. 492) but
    // I'm using round half up to 3 decimals.

    Boxes boxes{0.010, 0, BoxScale::e_Percent};

    Boxes::Box box = boxes.FindBox(500);
    EXPECT_EQ(box, Decimal("500.00"));

    box = boxes.FindBox(506);
    EXPECT_EQ(box, Decimal("505.00"));

    box = boxes.FindBox(511);
    EXPECT_EQ(box, Decimal("510.050"));

    box = boxes.FindBox(516);
    // EXPECT_EQ(box, Decimal("515.151"));
    EXPECT_EQ(box, Decimal("515.150"));  // mpdecimal rounding is different

    box = boxes.FindBox(532);
    EXPECT_EQ(box, Decimal("530.760"));

    box = boxes.FindBox(520);
    // EXPECT_EQ(box, Decimal("515.151"));
    EXPECT_EQ(box, Decimal("515.150"));  // mpdecimal rounding is different

    std::cout << boxes << std::endl;
    // test going smaller

    Boxes boxes2{0.01, 0.0, BoxScale::e_Percent};

    box = boxes2.FindBox(505);
    EXPECT_EQ(box, 505);

    box = boxes2.FindBox(500);
    EXPECT_EQ(box, Decimal("499.95"));
}

TEST_F(BoxesBasicFunctionality, PercentBoxesNextandPrev)  // NOLINT
{
    Boxes boxes{0.01, 0.0, BoxScale::e_Percent};
    Boxes::Box box = boxes.FindBox(500);
    EXPECT_EQ(box, Decimal("500.00"));

    box = boxes.FindNextBox(500);
    EXPECT_EQ(box, 505);

    Boxes boxes2{0.01, 0.0, BoxScale::e_Percent};
    box = boxes2.FindBox(500);
    box = boxes2.FindBox(505);
    EXPECT_EQ(box, 505);

    box = boxes2.FindPrevBox(505);
    EXPECT_EQ(box, 500);
}

TEST_F(BoxesBasicFunctionality, BoxesToAndFromJson)  // NOLINT
{
    const std::string data = "500.0 505.0 510.05 515.151 520.303 525.506 530.761";

    auto values = rng_split_string<std::string>(data, " ");

    auto prices = values | vws::transform(
                               [](const auto& a_value)
                               {
                                   Decimal result{a_value};
                                   return result;
                               });

    Boxes boxes{0.01, 0.0, BoxScale::e_Percent};

    rng::for_each(prices, [&boxes](const auto& x) { boxes.FindBox(x); });

    const auto json = boxes.ToJSON();

    Boxes boxes2{json};

    ASSERT_EQ(boxes, boxes2);
}

TEST_F(BoxesBasicFunctionality, BoxesToJsonThenFromJsonThenAddData)  // NOLINT
{
    // first, construct a Boxes using complete set of data

    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122,
                                         1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128,
                                         1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    Boxes boxes{Decimal{10}};

    rng::for_each(prices, [&boxes](const auto& x) { boxes.FindBox(x); });

    // next, construct using part of the data, save to JSON, then load from JSON and add the remaining data

    const std::vector<int32_t> prices_1 = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                           1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};
    const std::vector<int32_t> prices_2 = {1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111,
                                           1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    Boxes boxes_1{Decimal{10}};
    rng::for_each(prices_1, [&boxes_1](const auto& x) { boxes_1.FindBox(x); });
    const auto json_1 = boxes_1.ToJSON();

    Boxes boxes_2{json_1};
    rng::for_each(prices_2, [&boxes_2](const auto& x) { boxes_2.FindBox(x); });

    ASSERT_EQ(boxes, boxes_2);
}

class Combinatorial : public Test
{
};

TEST_F(Combinatorial, BasicFunctionlity)  // NOLINT
{
    std::vector<Decimal> a = {1, Decimal("2.0"), Decimal("3.5")};
    std::vector<char> b = {'a', 'c'};
    std::vector<std::string_view> c = {"def", "hij", "mnop"};

    auto abc = vws::cartesian_product(a, b, c);
    //    rng::for_each(abc, [](const auto& x) {std::print("{}\n", x); });

    //    std::print("{}\n", abc);

    ASSERT_EQ(rng::size(abc), 18);
}

class ColumnFunctionality10X1 : public Test
{
};

TEST_F(ColumnFunctionality10X1, Constructors)  // NOLINT
{
    PF_Column col;

    ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ColumnFunctionality10X1, Equality)  // NOLINT
{
    PF_Column col1;

    EXPECT_EQ(col1.GetDirection(), PF_Column::Direction::e_Unknown);

    PF_Column col2;
    EXPECT_EQ(col1, col2);

    Boxes boxes{Decimal{5}};
    PF_Column col3{&boxes, 0, 5};
    ASSERT_NE(col3, col2);
}

TEST_F(ColumnFunctionality10X1, InitialColumnConstructionInitialValueAndDirection)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    auto a_value = prices.begin();

    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    //    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(Decimal{*a_value}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    // std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(Decimal{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    // std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(Decimal{*(++a_value)}, the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(Decimal(*a_value), the_time);
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X1, ContinueUntilFirstReversal)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    PF_Column::Status status;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    rng::for_each(prices,
                  [&col, &status, &the_time](auto price) { status = col.AddValue(Decimal(price), the_time).first; });
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1100);
    ASSERT_EQ(status, PF_Column::Status::e_Reversal);
}

TEST_F(ColumnFunctionality10X1, ContinueUntilFirstReversalThenJSON)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    PF_Column::Status status;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();
    //    std::cout << "time: " << the_time << '\n';

    rng::for_each(prices,
                  [&col, &status, &the_time](auto price) { status = col.AddValue(Decimal(price), the_time).first; });

    auto json = col.ToJSON();
    //   std::cout << json << '\n';

    //    EXPECT_EQ(json["box_size"], "10");

    // this is be the way to convert our stored time to a time_point.
    // we use nanoseconds because that is what tiingo provides in its
    // streaming interface.

    PF_Column::TmPt z{std::chrono::nanoseconds{json["first_entry"].asInt64()}};
    //    std::cout << "z: " << z << '\n';

    EXPECT_TRUE(z == the_time);

    //    std::cout << std::chrono::milliseconds(std::stol(json["first_entry"])) << ;\n';

    EXPECT_EQ(json["direction"].asString(), "up");
    EXPECT_EQ(json["top"].asString(), "1130");
    EXPECT_EQ(json["bottom"].asString(), "1100");
    ASSERT_EQ(status, PF_Column::Status::e_Reversal);
}

TEST_F(ColumnFunctionality10X1, ColumnToJsonThenFromJsonThenAddData)  // NOLINT
{
    // first, construct a Boxes using complete set of data

    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    PF_Column::Status status;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    rng::for_each(prices,
                  [&col, &status, &the_time](auto price) { status = col.AddValue(Decimal(price), the_time).first; });

    // next, construct using part of the data, save to JSON, then load from JSON and add the remaining data

    const std::vector<int32_t> prices_1 = {1100, 1105, 1110, 1112};
    const std::vector<int32_t> prices_2 = {1118, 1120, 1136, 1121, 1129, 1120};

    Boxes boxes_1{Decimal{10}};
    PF_Column col_1{&boxes_1, 0, 1};
    rng::for_each(prices_1, [&col_1, &status, &the_time](auto price)
                  { status = col_1.AddValue(Decimal(price), the_time).first; });

    const auto json_1 = boxes_1.ToJSON();
    const auto col_1_json = col_1.ToJSON();

    Boxes boxes_2{json_1};
    PF_Column col_2 = {&boxes_2, col_1_json};
    rng::for_each(prices_2, [&col_2, &status, &the_time](auto price)
                  { status = col_2.AddValue(Decimal(price), the_time).first; });

    //    std::cout << "\n\n col:\n" << col;
    //    std::cout << "\n\n col_1:\n" << col_1;
    //    std::cout << "\n\n col_2:\n" << col_2 << '\n';;

    EXPECT_EQ(boxes, boxes_2);
    ASSERT_EQ(col, col_2);
}

TEST_F(ColumnFunctionality10X1, ConstructValueStoreAsJSONThenConstructCopy)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    PF_Column::Status status;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    rng::for_each(prices,
                  [&col, &status, &the_time](auto price) { status = col.AddValue(Decimal(price), the_time).first; });

    auto json = col.ToJSON();
    //    std::cout << json << '\n';

    PF_Column col2{&boxes, json};
    EXPECT_EQ(col, col2);
    EXPECT_EQ(col.GetTimeSpan(), col2.GetTimeSpan());
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversal)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    std::vector<PF_Column> columns;

    PF_Column::TmPt the_time = std::chrono::utc_clock::now();
    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        //        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: "
        //        << col.GetBottom() <<
        //        '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
            //            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  "
            //            bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(columns.back().GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(columns.back().GetTop(), 1130);
    EXPECT_EQ(columns.back().GetBottom(), 1100);

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1120);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedByOneStepBack)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        //        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: "
        //        << col.GetBottom() <<
        //        '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
            //            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  "
            //            bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1130);
    EXPECT_EQ(col.GetBottom(), 1120);
    EXPECT_EQ(col.GetHadReversal(), true);
    EXPECT_EQ(columns.size() + 1, 2);
}

TEST_F(ColumnFunctionality10X1, ProcessFirst1BoxReversalFollowedBySeriesOfOneStepBacks)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120,
                                         1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128, 1136, 1111};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        //        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: "
        //        << col.GetBottom() <<
        //        '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
            //            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  "
            //            bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1120);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 4);

    // rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstHalfOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        //        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: "
        //        << col.GetBottom() <<
        //        '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
            //            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  "
            //            bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1110);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 5);

    // rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X1, ProcessCompletelyFirstSetOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122,
                                         1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128,
                                         1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        //        std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  " bottom: "
        //        << col.GetBottom() <<
        //        '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
            //            std::cout << "price: " << price << " result: " << status << " top: " << col.GetTop() <<  "
            //            bottom: " << col.GetBottom() << '\n';
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), 1140);
    EXPECT_EQ(col.GetBottom(), 1130);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 9);

    // rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

class ColumnFunctionalityFractionalBoxes10X1 : public Test
{
};

TEST_F(ColumnFunctionalityFractionalBoxes10X1, Constructors)  // NOLINT
{
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};

    ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ColumnFunctionalityFractionalBoxes10X1, InitialColumnConstructionInitialValueAndDirection)  // NOLINT
{
    const std::vector<double> prices = {1100.4, 1105.9, 1110.3, 1112.2, 1118.7, 1120.6};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 1};
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    auto a_value = prices.begin();

    //    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(dbl2dec(*a_value), the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    //    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(dbl2dec(*(++a_value)), the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    //    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(dbl2dec(*(++a_value)), the_time);
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(dbl2dec(*a_value), the_time);
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);

    // std::cout << "col: " << col << '\n';
}

class ColumnFunctionality10X3 : public Test
{
};

TEST_F(ColumnFunctionality10X3, Constructors)  // NOLINT
{
    PF_Column col;

    ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ColumnFunctionality10X3, InitialColumnConstructionInitialValueAndDirection)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120};
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 3};

    auto a_value = prices.begin();

    PF_Column::TmPt the_time = std::chrono::utc_clock::now();
    //    std::cout << "first value: " << *a_value << '\n';
    auto status = col.AddValue(Decimal{*a_value}, the_time);
    //    std::cout << "first value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  "
    //    bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    //    std::cout << "second value: " << *(++a_value) << '\n';
    status = col.AddValue(Decimal{*(++a_value)}, the_time);
    //    std::cout << "second value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  "
    //    bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_Ignored);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
    EXPECT_EQ(col.GetTop(), 1100);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    //    std::cout << "third value: " << *(++a_value) << '\n';
    status = col.AddValue(Decimal{*(++a_value)}, the_time);
    //    std::cout << "third value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<  "
    //    bottom: " << col.GetBottom() << '\n';
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1110);
    EXPECT_EQ(col.GetBottom(), 1100);

    the_time = std::chrono::utc_clock::now();
    while (++a_value != prices.end())
    {
        status = col.AddValue(Decimal(*a_value), the_time);
        //        std::cout << "next value: " << *a_value << " result: " << status.first << " top: " << col.GetTop() <<
        //        " bottom: " << col.GetBottom() << '\n';
    }
    EXPECT_EQ(status.first, PF_Column::Status::e_Accepted);
    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
}

TEST_F(ColumnFunctionality10X3, ProcessFirstHalfOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129};

    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 3};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), 1120);
    EXPECT_EQ(col.GetBottom(), 1100);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 2);

    //    rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X3, ProcessCompletelyFirstSetOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122,
                                         1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128,
                                         1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 3};

    std::vector<PF_Column> columns;

    PF_Column::TmPt the_time = std::chrono::utc_clock::now();
    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1150);
    EXPECT_EQ(col.GetBottom(), 1110);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 3);

    // rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

class ColumnFunctionality10X5 : public Test
{
};

TEST_F(ColumnFunctionality10X5, Constructors)  // NOLINT
{
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 5};

    ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ColumnFunctionality10X5, ProcessCompletelyFirstSetOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122,
                                         1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128,
                                         1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 5};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), 1150);
    EXPECT_EQ(col.GetBottom(), 1100);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 1);

    rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

class ColumnFunctionality10X2 : public Test
{
};

TEST_F(ColumnFunctionality10X2, Constructors)  // NOLINT
{
    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 2};

    ASSERT_EQ(col.GetDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestData)  // NOLINT
{
    const std::vector<int32_t> prices = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129,
                                         1138, 1113, 1139, 1123, 1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122,
                                         1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127, 1138, 1111, 1122, 1111, 1128,
                                         1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    Boxes boxes{Decimal{10}};
    PF_Column col{&boxes, 0, 2};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (auto price : prices)
    {
        auto [status, new_col] = col.AddValue(Decimal(price), the_time);
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(Decimal(price), the_time).first;
        }
    }

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), 1140);
    EXPECT_EQ(col.GetBottom(), 1130);
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 6);

    // rng::for_each(columns, [](const auto& a_col) { std::cout << a_col << '\n'; });
    //    std::cout << col << '\n';
}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestDataWithATRFractionalBoxSize)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    auto value_differences =
        values_ints | vws::slide(2) | vws::transform([](const auto x) { return abs(x[1] - x[0]); });

    // do a simplified ATR calculation

    Decimal box_size{
        dbl2dec(static_cast<double>(std::accumulate(value_differences.begin(), value_differences.end(), 0)) /
                static_cast<double>(value_differences.size()))};
    box_size = box_size.rescale(-5);

    EXPECT_EQ(box_size, Decimal("12.91837"));

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    Boxes boxes{box_size};
    PF_Column col{&boxes, 0, 2};

    std::vector<PF_Column> columns;

    auto data_values = rng_split_string<std::string_view>(test_data, "\n");
    int close_col = 1;
    int date_col = 0;
    for (const auto& record : data_values)
    {
        if (record.empty())
        {
            continue;
        }
        const auto fields = split_string<std::string_view>(record, ",");
        auto [status, new_col] =
            col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col])).first;
        }
    };

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(col.GetTop(), Decimal("1151.672"));
    EXPECT_EQ(col.GetBottom(), Decimal("1125.836"));
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 5);
}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestDataWithFractionalBoxSizeAndPercent)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    Boxes boxes{0.01, 0.0, BoxScale::e_Percent};
    PF_Column col{&boxes, 0, 2};

    std::vector<PF_Column> columns;

    auto data_values = rng_split_string<std::string_view>(test_data, "\n");
    int close_col = 1;
    int date_col = 0;
    for (const auto& record : data_values)
    {
        if (record.empty())
        {
            continue;
        }
        const auto fields = split_string<std::string_view>(record, ",");
        auto [status, new_col] =
            col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));

        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col])).first;
        }
    };

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), Decimal("1144.664"));
    EXPECT_EQ(col.GetBottom(), Decimal("1133.331"));
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 8);
}

TEST_F(ColumnFunctionality10X2, ProcessCompletelyFirstSetOfTestDataWithATRFractionalBoxSizeAndPercent)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    // compute percent box size based on simplified ATR computation
    const auto value_differences =
        values_ints | vws::slide(2) | vws::transform([](const auto x) { return abs(x[1] - x[0]); });

    Decimal atr =
        dbl2dec(static_cast<double>(std::accumulate(value_differences.begin(), value_differences.end(), 0.0)) /
                static_cast<double>(value_differences.size()));
    // Decimal average_price = static_cast<double>(rng::accumulate(values_ints, 0.0)) /
    // static_cast<double>(values_ints.size()); Decimal box_size = atr / average_price; box_size.Rescale(-5);

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    Boxes boxes{atr, Decimal("0.01"), BoxScale::e_Percent};
    PF_Column col{&boxes, 0, 2};

    std::vector<PF_Column> columns;

    auto data_values = rng_split_string<std::string_view>(test_data, "\n");
    int close_col = 1;
    int date_col = 0;
    for (const auto& record : data_values)
    {
        if (record.empty())
        {
            continue;
        }
        const auto fields = split_string<std::string_view>(record, ",");
        auto [status, new_col] =
            col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));

        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col])).first;
        }
    };

    EXPECT_EQ(col.GetDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(col.GetTop(), Decimal("1144.664"));
    EXPECT_EQ(col.GetBottom(), Decimal("1133.331"));
    EXPECT_EQ(col.GetHadReversal(), false);
    EXPECT_EQ(columns.size() + 1, 8);
}

class ColumnFunctionalityPercentX1 : public Test
{
};

TEST_F(ColumnFunctionalityPercentX1, SimpleAscendingData)  // NOLINT
{
    const std::string data = "500.0 505.0 510.05 515.151 520.303 525.506 530.761";

    auto values = rng_split_string<std::string>(data, " ");

    auto prices = values | vws::transform(
                               [](const auto& a_value)
                               {
                                   Decimal result{a_value};
                                   return result;
                               });
    //    rng::for_each(values, [](const auto& x) { std::cout << x << "  "; });
    //    std::cout << '\n';

    Boxes boxes{0.01, 0.0, BoxScale::e_Percent};
    PF_Column col{&boxes, 0, 2};

    std::vector<PF_Column> columns;
    PF_Column::TmPt the_time = std::chrono::utc_clock::now();

    for (const auto& price : prices)
    {
        auto [status, new_col] = col.AddValue(price, the_time);
        //        std::cout << "value: " << price << " status: " << status << " top: " << col.GetTop() << '\n';
        if (status == PF_Column::Status::e_Reversal)
        {
            columns.push_back(col);
            col = std::move(new_col.value());

            // now continue on processing the value.

            status = col.AddValue(price, the_time).first;
        }
    }

    //    std::cout << "Column: " << col << '\n';

    // EXPECT_EQ(col.GetTop(), Decimal("530.761"));
    EXPECT_EQ(col.GetTop(), Decimal("530.760"));  // mpdecimal rounds differently
    EXPECT_EQ(col.GetBottom(), Decimal("500.00"));
}

class ChartFunctionality10X2 : public Test
{
};

TEST_F(ChartFunctionality10X2, Constructors)  // NOLINT
{
    PF_Chart chart("GOOG", 10, 2);

    ASSERT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Unknown);
}

TEST_F(ChartFunctionality10X2, EmptyChartToJSON)  // NOLINT
{
    PF_Chart chart("GOOG", 10, 2);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Unknown);

    auto json = chart.ToJSON();
    //   std::cout << json << '\n';

    ASSERT_EQ(json["current_direction"].asString(), "unknown");
}

TEST_F(ChartFunctionality10X2, EmptyChartToAndFromJSON)  // NOLINT
{
    PF_Chart chart("GOOG", 10, 2);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Unknown);

    auto json = chart.ToJSON();
    //   std::cout << json << '\n';

    EXPECT_EQ(json["current_direction"].asString(), "unknown");

    PF_Chart chart2{json};
    ASSERT_EQ(chart, chart2);
}

TEST_F(ChartFunctionality10X2, StartWithSimpleTestData)  // NOLINT
{
    PF_Chart chart("AAPL", 10, 2);

    chart.AddValue("100", "2023-06-01", "%Y-%m-%d");

    std::cout << "after first value: " << chart << std::endl;

    chart.AddValue("100", "2023-06-02", "%Y-%m-%d");

    std::cout << "after second value: s.b. ignored" << chart << std::endl;
}

TEST_F(ChartFunctionality10X2, ProcessCompletelyFirstSetOfTestData)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);
}

TEST_F(ChartFunctionality10X2, TestChartIteratorWithFirstSetOfTestData)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), rng::size(chart));

    std::vector<int> a = {0, 1, 2, 3, 4, 5};
    auto col_nbrs =
        vws::transform(chart, [](const auto& col) { return col.GetColumnNumber(); }) | ranges::to<std::vector>();
    EXPECT_EQ(col_nbrs, a);

    std::vector<int> b = {5, 4, 3, 2, 1, 0};
    auto col_nbrs_r = vws::transform(chart | vws::reverse, [](const auto& col) { return col.GetColumnNumber(); }) |
                      ranges::to<std::vector>();
    EXPECT_EQ(col_nbrs_r, b);

    EXPECT_EQ(chart.begin()[5].GetTop(), 1140);
    EXPECT_EQ((chart.begin() + 5)->GetBottom(), 1130);
    EXPECT_EQ((chart.begin() += 5)->GetHadReversal(), false);
}

TEST_F(ChartFunctionality10X2, TestChartReverseIteratorWithFirstSetOfTestData)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    // EXPECT_EQ(chart.size(), rng::size(rng::subrange(chart.rbegin(), chart.rend())));

    std::vector<int> b = {5, 4, 3, 2, 1, 0};
    auto col_nbrs_r = vws::transform(rng::subrange(chart.rbegin(), chart.rend()),
                                     [](const auto& col) { return col.GetColumnNumber(); }) |
                      ranges::to<std::vector>();
    EXPECT_EQ(col_nbrs_r, b);
    //    // rng::for_each(chart.rbegin(), chart.rend(), [](const auto& col) { std::print("col: {}\n",
    //    col.GetColumnNumber()); });
    //
    std::vector<int> a = {0, 1, 2, 3, 4, 5};
    auto col_nbrs = vws::transform(rng::subrange(chart.rbegin(), chart.rend()) | vws::reverse,
                                   [](const auto& col) { return col.GetColumnNumber(); }) |
                    ranges::to<std::vector>();
    EXPECT_EQ(col_nbrs, a);

    //    std::cout << chart << '\n';

    EXPECT_EQ(chart.rbegin()[5].GetTop(), 1130);
    EXPECT_EQ((chart.rbegin() + 5)->GetBottom(), 1100);
    EXPECT_EQ((chart.rbegin() += 5)->GetHadReversal(), false);
}

TEST_F(ChartFunctionality10X2, ProcessSomeDataThenToJSONThenFromJSONThenMoreData)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});
    // std::cout << "test data: " << test_data << std::endl;

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    const std::vector<int32_t> values_ints_1 = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129,
                                                1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128,
                                                1136, 1111, 1095, 1102, 1108, 1092, 1129};
    const std::vector<int32_t> values_ints_2 = {1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
                                                1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119,
                                                1132, 1133, 1147, 1131, 1159, 1136, 1127};
    std::string test_data_1 = MakeSimpleTestData(
        values_ints_1, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});
    // dates used for second data set need to be non-overlapping with first because Chart code now filters 'old' data.
    std::string test_data_2 = MakeSimpleTestData(
        values_ints_2, std::chrono::year_month_day{2016y / std::chrono::March / std::chrono::Monday[1]});

    std::istringstream prices_1{test_data_1};

    PF_Chart chart_1("GOOG", 10, 2);
    chart_1.LoadData(&prices_1, "%Y-%m-%d", ",");

    const auto chart_1_json = chart_1.ToJSON();

    PF_Chart chart_2{chart_1_json};
    std::istringstream prices_2{test_data_2};
    chart_2.LoadData(&prices_2, "%Y-%m-%d", ",");

    // std::cout << "\n\n chart:\n" << chart;
    // std::cout << "\n\n chart_1:\n" << chart_1;
    // std::cout << "\n\n chart_2:\n" << chart_2 << '\n';;

    ASSERT_EQ(chart, chart_2);
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsInts)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    //    PF_Chart chart("AAPL", 2, 2, BoxType::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(chart.size(), 47);

    EXPECT_EQ(chart[46].GetTop(), 148);
    EXPECT_EQ(chart[46].GetBottom(), 146);

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick12.svg", {}, "no");
    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsToJSON)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    // PF_Chart chart("AAPL", 2, 2, BoxType::e_fractional);
    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    auto json = chart.ToJSON();

    EXPECT_EQ(json["current_direction"].asString(), "up");
    EXPECT_EQ(json["columns"].size(), 46);

    EXPECT_EQ(json["columns"][45]["top"].asString(), "146");
    EXPECT_EQ(json["columns"][45]["bottom"].asString(), "144");

    //    std::cout << chart << '\n';
}

TEST_F(ChartFunctionality10X2, ProcessFileWithFractionalDataButUseAsIntsToJSONFromJSON)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2);
    //    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    auto json = chart.ToJSON();

    PF_Chart chart2{json};
    ASSERT_EQ(chart, chart2);

    //    std::cout << chart << '\n';
}

// use ATR computed box size instead of predefined box size

class ChartFunctionalitySimpleATRX2 : public Test
{
};

TEST_F(ChartFunctionalitySimpleATRX2, ComputeATRBoxSizeForFirstSetOfTestData)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    // compute a 'simple' ATR by taking successive differences and using that as the true range then compute the ATR
    // using those values.

    const auto value_differences =
        values_ints | vws::slide(2) | vws::transform([](const auto x) { return abs(x[1] - x[0]); });
    //    rng::for_each(value_differences, [](const auto& x) { std::cout << x << "  "; });
    //    std::cout << '\n';

    EXPECT_EQ(value_differences[0], 5);
    EXPECT_EQ(value_differences[6], 15);
    EXPECT_EQ(value_differences[value_differences.size() - 1], 9);

    Decimal atr = dbl2dec(static_cast<double>(std::accumulate(value_differences.begin(), value_differences.end(), 0)) /
                          static_cast<double>(value_differences.size()));
    atr = atr.rescale(-5);
    //    std::cout << "atr: " << atr << '\n';

    EXPECT_EQ(atr, Decimal("12.91837"));
}

TEST_F(ChartFunctionalitySimpleATRX2, ProcessCompletelyFirstSetOfTestDataWithATRAndFractionalBoxsize)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    const auto value_differences =
        values_ints | vws::slide(2) | vws::transform([](const auto x) { return abs(x[1] - x[0]); });

    Decimal atr = dbl2dec(static_cast<double>(std::accumulate(value_differences.begin(), value_differences.end(), 0)) /
                          static_cast<double>(value_differences.size()));
    atr = atr.rescale(-5);
    //    std::cout << "atr: " << atr << '\n';

    EXPECT_EQ(atr, Decimal("12.91837"));

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    PF_Chart chart("GOOG", atr, 2, 0, BoxScale::e_Linear);

    // do it manually so can watch chart formation

    auto data_values = rng_split_string<std::string_view>(test_data, "\n");
    int close_col = 1;
    int date_col = 0;
    for (const auto& record : data_values)
    {
        if (record.empty())
        {
            continue;
        }
        const auto fields = split_string<std::string_view>(record, ",");
        auto result = chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
    };

    // std::print("{}\n", chart);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(chart.size(), 5);
    EXPECT_EQ(chart[4].GetTop(), Decimal("1151.672"));
    EXPECT_EQ(chart[4].GetBottom(), Decimal("1125.836"));
    EXPECT_EQ(chart[4].GetHadReversal(), false);
}

class MiscChartFunctionality : public Test
{
};

TEST_F(MiscChartFunctionality, TestChartIterators)  // NOLINT
{
    const std::string data =
            "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
            "1108 1092 1129 "
            "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
            "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    // start with 1 box reversal - lots of short columns

    PF_Chart chart1("GOOG", 10, 1);
    chart1.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart1.size(), 9);
    EXPECT_EQ(rng::distance(chart1), 9);

    prices.clear();
    prices.seekg(0);
    PF_Chart chart5("GOOG", 10, 5);
    chart5.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart5.size(), 1);
    EXPECT_EQ(rng::distance(chart5), 1);
}

TEST_F(MiscChartFunctionality, TestChartBoxFiltersWithBoxes)  // NOLINT
{
    const std::string data =
            "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
            "1108 1092 1129 "
            "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
            "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    // start with 1 box reversal - lots of short columns

    PF_Chart chart1("GOOG", 10, 1);
    chart1.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart1.size(), 9);
    EXPECT_EQ(chart1.GetBoxesForColumns(PF_ColumnFilter::e_up_column).size(), 9);
    EXPECT_EQ(chart1.GetBoxesForColumns(PF_ColumnFilter::e_reversed_to_up).size(), 8);

    prices.clear();
    prices.seekg(0);
    PF_Chart chart5("GOOG", 10, 5);
    chart5.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart5.size(), 1);
    EXPECT_EQ(chart5.GetBoxesForColumns(PF_ColumnFilter::e_up_column).size(), 6);
    EXPECT_EQ(chart5.GetBoxesForColumns(PF_ColumnFilter::e_reversed_to_down).size(), 0);
}

TEST_F(MiscChartFunctionality, TestChartBoxFiltersWithColumns)  // NOLINT
{
    const std::string data =
            "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
            "1108 1092 1129 "
            "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
            "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    // start with 1 box reversal - lots of short columns

    PF_Chart chart1("GOOG", 10, 1);
    chart1.LoadData(&prices, "%Y-%m-%d", ",");

    std::cout << chart1 << std::endl;

    EXPECT_EQ(chart1.size(), 9);
    EXPECT_EQ(chart1.GetTopBottomForColumns(PF_ColumnFilter::e_up_column).size(), 3);
    const auto top_bottom = chart1.GetTopBottomForColumns(PF_ColumnFilter::e_reversed_to_up);
    EXPECT_EQ(top_bottom.size(), 3);
    // EXPECT_EQ(std::get<2>(top_bottom[2]), 1160);

    prices.clear();
    prices.seekg(0);
    PF_Chart chart5("GOOG", 10, 5);
    chart5.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart5.size(), 1);
    EXPECT_EQ(chart5.GetTopBottomForColumns(PF_ColumnFilter::e_up_column).size(), 1);
    EXPECT_EQ(chart5.GetTopBottomForColumns(PF_ColumnFilter::e_reversed_to_down).size(), 0);
}
TEST_F(MiscChartFunctionality, LoadDataFromJSONChartFileThenAddDataFromCSV)  // NOLINT
{
    fs::path symbol_file_name{"./test_files/SPY_1.json"};

    PF_Chart new_chart = PF_Chart::MakeChartFromJSONFile(symbol_file_name);

    //    std::cout << new_chart << '\n';

    fs::path csv_file_name{"./test_files3/SPY.csv"};
    const std::string file_content_csv = LoadDataFileForUse(csv_file_name);

    const auto symbol_data_records = split_string<std::string_view>(file_content_csv, "\n");
    const auto header_record = symbol_data_records.front();

    auto date_column = FindColumnIndex(header_record, "date", ",");
    BOOST_ASSERT_MSG(date_column.has_value(),
                     std::format("Can't find 'date' field in header record: {}.", header_record).c_str());

    auto close_column = FindColumnIndex(header_record, "Close", ",");
    BOOST_ASSERT_MSG(close_column.has_value(),
                     std::format("Can't find price field: 'Close' in header record: {}.", header_record).c_str());

    //    std::cout << "new chart at start of adding new data: \n\n" << new_chart << "\n\n";
    rng::for_each(symbol_data_records | vws::drop(1),
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record)
                  {
                      //            std::cout << "len: " << record.size() << "  " << record << '\n';
                      const auto fields = split_string<std::string_view>(record, ",");
                      //            std::cout << "close value: " << fields[close_col] << " date value: " <<
                      //            fields[date_col] << " record: \n" << record << '\n';
                      new_chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
                  });
    //    std::cout << "new chart at AFTER adding new data: \n\n" << new_chart << "\n\n";
}

TEST_F(MiscChartFunctionality, LoadDataFromCSVFileThenAddDataFromPricesDB)  // NOLINT
{
    if (fs::exists("/tmp/candlestick5.svg"))
    {
        fs::remove("/tmp/candlestick5.svg");
    }
    if (fs::exists("/tmp/candlestick6.svg"))
    {
        fs::remove("/tmp/candlestick6.svg");
    }

    fs::path csv_file_name{"./test_files/SPY.csv"};
    const std::string file_content_csv = LoadDataFileForUse(csv_file_name);

    const auto symbol_data_records = split_string<std::string_view>(file_content_csv, "\n");
    const auto header_record = symbol_data_records.front();

    auto date_column = FindColumnIndex(header_record, "date", ",");
    BOOST_ASSERT_MSG(date_column.has_value(),
                     std::format("Can't find 'date' field in header record: {}.", header_record).c_str());

    auto close_column = FindColumnIndex(header_record, "Close", ",");
    BOOST_ASSERT_MSG(close_column.has_value(),
                     std::format("Can't find price field: 'Close' in header record: {}.", header_record).c_str());

    PF_Chart new_chart{"SPY", 10, 1};

    rng::for_each(symbol_data_records | vws::drop(1),
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record)
                  {
                      const auto fields = split_string<std::string_view>(record, ",");
                      new_chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
                  });
    //    std::cout << "new chart at after loading initial data: \n\n" << new_chart << "\n\n";

    ConstructCDChartGraphicAndWriteToFile(new_chart, "/tmp/candlestick5.svg", {}, "no");

    // save for comparison

    PF_Chart chart2 = new_chart;

    // load updates from prices DB

    struct DB_data
    {
        std::chrono::utc_time<std::chrono::utc_clock::duration> tp;
        Decimal price;
    };
    std::vector<DB_data> db_data;

    // first, get ready to retrieve our data from DB.  Do this for all our symbols here.
    // (we know what data to extract because we know what is in our CSV original source above)

    std::string get_symbol_prices_cmd =
        "SELECT date, split_adj_close FROM new_stock_data.current_data WHERE symbol = 'SPY' AND date >= '2020-03-26' "
        "ORDER BY date ASC";

    try
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::transaction trxn{c};  // we are read-only for this work

        // we know our database contains 'date's, but we need timepoints

        std::istringstream time_stream;
        date::utc_time<std::chrono::utc_clock::duration> tp;

        for (const auto& [date, split_adj_close] :
             trxn.stream<std::string_view, std::string_view>(get_symbol_prices_cmd))
        {
            time_stream.clear();
            time_stream.str(std::string{date});
            date::from_stream(time_stream, "%F", tp);
            std::chrono::utc_time<std::chrono::utc_clock::duration> tp1{tp.time_since_epoch()};
            db_data.emplace_back(DB_data{.tp = tp1, .price = sv2dec(split_adj_close)});
        }
        trxn.commit();

        std::cout << "done retrieving data for symbol SPY. Got: " << db_data.size() << " rows." << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Unable to load data for SPY because: " << e.what() << std::endl;
    }

    rng::for_each(db_data, [&new_chart](const auto& row) { new_chart.AddValue(row.price, row.tp); });
    //    std::cout << "new chart at AFTER loading new data: \n\n" << new_chart << "\n\n";

    ConstructCDChartGraphicAndWriteToFile(new_chart, "/tmp/candlestick6.svg", {}, "no");

    EXPECT_NE(new_chart, chart2);
}

TEST_F(MiscChartFunctionality, LoadDataFromCSVFileThenMakeChartThenExportCSV)  // NOLINT
{
    if (fs::exists("/tmp/SPY_chart.csv"))
    {
        fs::remove("/tmp/SPY_chart.csv");
    }
    if (fs::exists("/tmp/SPY_chart.svg"))
    {
        fs::remove("/tmp/SPY_chart.svg");
    }

    fs::path csv_file_name{"./test_files/SPY.csv"};
    const std::string file_content_csv = LoadDataFileForUse(csv_file_name);

    const auto symbol_data_records = split_string<std::string_view>(file_content_csv, "\n");
    const auto header_record = symbol_data_records.front();

    auto date_column = FindColumnIndex(header_record, "date", ",");
    BOOST_ASSERT_MSG(date_column.has_value(),
                     std::format("Can't find 'date' field in header record: {}.", header_record).c_str());

    auto close_column = FindColumnIndex(header_record, "Close", ",");
    BOOST_ASSERT_MSG(close_column.has_value(),
                     std::format("Can't find price field: 'Close' in header record: {}.", header_record).c_str());

    PF_Chart new_chart{"SPY", 10, 1};

    rng::for_each(symbol_data_records | vws::drop(1),
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record)
                  {
                      const auto fields = split_string<std::string_view>(record, ",");
                      new_chart.AddValue(fields[close_col], fields[date_col], "%Y-%m-%d");
                  });
    //    std::cout << "new chart at after loading initial data: \n\n" << new_chart << "\n\n";

    std::ofstream processed_data{"/tmp/SPY_chart.csv"};
    new_chart.ConvertChartToTableAndWriteToStream(processed_data);
    ConstructCDChartGraphicAndWriteToFile(new_chart, "/tmp/SPY_chart.svg", {}, "no");

    ASSERT_TRUE(fs::exists("/tmp/SPY_chart.csv"));
}

TEST_F(MiscChartFunctionality, DontReloadOldData)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    PF_Chart saved_chart = chart;
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    ASSERT_EQ(chart, saved_chart);
}

TEST_F(MiscChartFunctionality, DontReloadOldDataButCanAddNewData)  // NOLINT
{
    const std::vector<int32_t> values_ints_1 = {1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129,
                                                1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123, 1128,
                                                1136, 1111, 1095, 1102, 1108, 1092, 1129};
    std::string test_data_1 = MakeSimpleTestData(
        values_ints_1, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    std::istringstream prices_1{test_data_1};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices_1, "%Y-%m-%d", ",");

    // EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    // EXPECT_EQ(chart.size(), 6);
    //
    // EXPECT_EQ(chart[5].GetTop(), 1140);
    // EXPECT_EQ(chart[5].GetBottom(), 1130);
    // EXPECT_EQ(chart[5].GetHadReversal(), false);

    PF_Chart saved_chart = chart;

    const std::vector<int32_t> values_ints_2 = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};
    std::string test_data_2 = MakeSimpleTestData(
        values_ints_2, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    std::istringstream prices_2{test_data_2};

    chart.LoadData(&prices_2, "%Y-%m-%d", ",");

    ASSERT_NE(chart, saved_chart);
}

TEST_F(MiscChartFunctionality, CheckColumnBoxCounts)  // NOLINT
{
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    // start with 1 box reversal - lots of short columns

    PF_Chart chart1("GOOG", 10, 1);
    chart1.LoadData(&prices, "%Y-%m-%d", ",");

    const std::vector<size_t> col_lens1 = {4, 2, 2, 3, 3, 2, 2, 4, 2};

    std::vector<size_t> found_lens1;
    rng::for_each(chart1, [&found_lens1] (const auto& col) { found_lens1.push_back(col.GetColumnBoxes().size()); });
    EXPECT_EQ(col_lens1, found_lens1);

    // next 5 box reversal - 1 long column

    prices.clear();
    prices.seekg(0);
    PF_Chart chart5("GOOG", 10, 5);
    chart5.LoadData(&prices, "%Y-%m-%d", ",");

    const std::vector<size_t> col_lens5 = {6};

    std::vector<size_t> found_lens5;
    rng::for_each(chart5, [&found_lens5] (const auto& col) { found_lens5.push_back(col.GetColumnBoxes().size()); });
    EXPECT_EQ(col_lens5, found_lens5);
}
// use ATR computed box size instead of predefined box size with logarithmic charts

// class ColumnFunctionalityLogX1 : public Test
//{
//
// };
//
class PercentChartFunctionalitySimpleATRX2 : public Test
{
};

TEST_F(PercentChartFunctionalitySimpleATRX2, ProcessCompletelyFirstSetOfTestDataWithATR)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};

    // compute a 'simple' ATR by taking successive differences and using that as the true range then compute the ATR
    // using those values.

    const auto value_differences =
        values_ints | vws::slide(2) | vws::transform([](const auto x) { return abs(x[1] - x[0]); });

    Decimal atr =
        dbl2dec(static_cast<double>(std::accumulate(value_differences.begin(), value_differences.end(), 0.0)) /
                static_cast<double>(value_differences.size()));
    // Decimal average_price = static_cast<double>(rng::accumulate(values_ints, 0.0)) /
    // static_cast<double>(values_ints.size());

    // Decimal box_size = atr / average_price;
    // box_size.Rescale(-5);

    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", atr, 2, Decimal(".01"), BoxScale::e_Percent);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    // std::print("Chart: {}\n", chart);

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 8);
    EXPECT_EQ(chart[8].GetTop(), Decimal("1144.664"));
    EXPECT_EQ(chart[8].GetBottom(), Decimal("1133.331"));
    EXPECT_EQ(chart[8].GetHadReversal(), false);
}

class ChartSignals10X3 : public Test
{
};

TEST_F(ChartSignals10X3, FindDoubleTopBuy)  // NOLINT
{
    const std::vector<int32_t> values_ints = {
        1100, 1105, 1110, 1112, 1118, 1120, 1136, 1121, 1129, 1120, 1139, 1121, 1129, 1138, 1113, 1139, 1123,
        1128, 1136, 1111, 1095, 1102, 1108, 1092, 1129, 1122, 1133, 1125, 1139, 1105, 1132, 1122, 1131, 1127,
        1138, 1111, 1122, 1111, 1128, 1115, 1117, 1120, 1119, 1132, 1133, 1147, 1131, 1159, 1136, 1127};
    std::string test_data = MakeSimpleTestData(
        values_ints, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]});

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 3);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    //    std::cout << chart << '\n';

    EXPECT_TRUE(!chart.GetSignals().empty());
    ASSERT_EQ(chart.GetSignals()[0].box_, 1140);
}

TEST_F(ChartSignals10X3, FindDoubleTopBuyAndDrawChart)  // NOLINT
{
    const fs::path csv_file_name{"./test_files/SPY_streaming_1min_2022-10-07.csv"};
    const std::string file_content_csv = LoadDataFileForUse(csv_file_name);

    const auto symbol_data_records = split_string<std::string_view>(file_content_csv, "\n");
    const auto header_record = symbol_data_records.front();

    auto date_column = FindColumnIndex(header_record, "date", ",");
    BOOST_ASSERT_MSG(date_column.has_value(),
                     std::format("Can't find 'date' field in header record: {}.", header_record).c_str());

    auto close_column = FindColumnIndex(header_record, "close", ",");
    BOOST_ASSERT_MSG(close_column.has_value(),
                     std::format("Can't find price field: 'Close' in header record: {}.", header_record).c_str());

    PF_Chart chart{"SPY", Decimal(".01"), 3};

    rng::for_each(symbol_data_records | vws::drop(1),
                  [&chart, close_col = close_column.value(), date_col = date_column.value()](const auto record)
                  {
                      const auto fields = split_string<std::string_view>(record, ",");
                      chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%F %X%z", fields[date_col]));
                  });
    // std::cout << "chart at after loading initial data: \n\n" << chart << "\n\n";

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick7.svg", {}, "no", PF_Chart::X_AxisFormat::e_show_time);

    // std::cout << chart << '\n';
    //
    EXPECT_TRUE(chart.GetSignals().size() > 0);
    // ASSERT_EQ(chart.GetSignals()[0].box_, 1140);
}

class TestDBFunctions : public Test
{
   public:
    void SetUp() override
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM test_point_and_figure.pf_charts");
        trxn.commit();
    }

    int CountRows()
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::nontransaction trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec1("SELECT count(*) FROM test_point_and_figure.pf_charts");
        trxn.commit();
        return row[0].as<int>();
    }

    int CountNYSESymbols()
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::nontransaction trxn{c};

        // make sure the DB is empty before we start

        auto row =
            trxn.exec1("SELECT COUNT(DISTINCT(symbol)) FROM new_stock_data.names_and_symbols WHERE exchange = 'NYSE'");
        trxn.commit();
        return row[0].as<int>();
    }
};

TEST_F(TestDBFunctions, TestRetrieveListOfExchangesInStocksDB)  // NOLINT
{
    PF_DB::DB_Params db_params{
        .user_name_ = "data_updater_pg", .db_name_ = "finance", .stock_db_data_source_ = "new_stock_data.current_data"};
    PF_DB pf_db{db_params};

    auto exchanges = pf_db.ListExchanges();

    ASSERT_EQ(exchanges, (std::vector<std::string>{"AMEX", "BATS", "INDX", "NASDAQ", "NMFQS", "NYSE", "OTC", "OTCCE",
                                                   "OTCGREY", "OTCMKTS", "OTCQB", "OTCQX", "PINK", "US"}));
}

TEST_F(TestDBFunctions, TestCountSymbolsOnNYSEExchange)  // NOLINT
{
    PF_DB::DB_Params db_params{
        .user_name_ = "data_updater_pg", .db_name_ = "finance", .stock_db_data_source_ = "new_stock_data.current_data"};
    PF_DB pf_db{db_params};

    const auto min_close_start_date_ =
        floor<std::chrono::days>(std::chrono::system_clock::now()) - std::chrono::days{183};
    auto symbols = pf_db.ListSymbolsOnExchange("NYSE", "5.00"s, 100'000);

    ASSERT_LT(symbols.size(), CountNYSESymbols());
}

class TestChartDBFunctions : public Test
{
   public:
    void SetUp() override
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        trxn.exec("DELETE FROM test_point_and_figure.pf_charts");
        trxn.commit();
    }

    int CountRows()
    {
        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::work trxn{c};

        // make sure the DB is empty before we start

        auto row = trxn.exec1("SELECT count(*) FROM test_point_and_figure.pf_charts");
        trxn.commit();
        return row[0].as<int>();
    }
};

TEST_F(TestChartDBFunctions, ProcessFileWithFractionalDataButUseAsIntsStoreInDB)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2);
    //    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    PF_DB::DB_Params db_params{.user_name_ = "data_updater_pg", .db_name_ = "finance", .PF_db_mode_ = "test"};
    PF_DB pf_db{db_params};

    pf_db.StorePFChartDataIntoDB(chart, "eod", {});

    auto how_many = CountRows();
    ASSERT_EQ(how_many, 1);

    //    std::cout << chart << '\n';
}

TEST_F(TestChartDBFunctions, ProcessFileWithFractionalDataButUseAsIntsStoreInDBThenRetrieveIntoJson)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name, std::ios::in | std::ios::binary};

    PF_Chart chart("AAPL", 2, 2);
    //    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    PF_DB::DB_Params db_params{.user_name_ = "data_updater_pg", .db_name_ = "finance", .PF_db_mode_ = "test"};
    PF_DB pf_db{db_params};

    pf_db.StorePFChartDataIntoDB(chart, "eod", {});

    // now, let's retrieve the stored data, construct a chart and
    // see if it's the same as the one we built directly fromt the data.

    PF_Chart chart2 = PF_Chart::MakeChartFromDB(pf_db, chart.GetChartParams(), "eod");

    //    std::cout << chart << '\n';
    EXPECT_EQ(chart, chart2);
}

TEST_F(TestChartDBFunctions, ProcessFileWithFractionalDataStoreInDBThenRetrieveIntoJson)  // NOLINT
{
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2);
    //    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    PF_DB::DB_Params db_params{.user_name_ = "data_updater_pg", .db_name_ = "finance", .PF_db_mode_ = "test"};
    PF_DB pf_db{db_params};

    pf_db.StorePFChartDataIntoDB(chart, "eod", {});

    // now, let's retrieve the stored data, construct a chart and
    // see if it's the same as the one we built directly fromt the data.

    PF_Chart chart2 = PF_Chart::MakeChartFromDB(pf_db, chart.GetChartParams(), "eod");
    ASSERT_EQ(chart, chart2);

    //    std::cout << chart << '\n';
}

TEST_F(TestChartDBFunctions, ComputeATRUsingDataFromDB)  // NOLINT
{
    // we should get the same result as we do from tiingo, I expect

    PF_DB::DB_Params db_params{
        .user_name_ = "data_updater_pg", .db_name_ = "finance", .stock_db_data_source_ = "new_stock_data.current_data"};
    PF_DB the_db{db_params};

    constexpr int history_size = 20;
    Decimal atr;

    try
    {
        auto price_data = the_db.RetrieveMostRecentStockDataRecordsFromDB("AAPL", StringToDateYMD("%F", "2021-10-07"),
                                                                          history_size + 1);
        atr = ComputeATR("AAPL", price_data, history_size);
    }
    catch (const std::exception& e)
    {
        std::cout << "Unable to comput ATR from DB for 'AAPL' because: " << e.what() << std::endl;
    }

    // results won't be exactly equal due to small differences in least
    // significant decimal digits of prices between tiingo and EODData
    EXPECT_EQ(atr.rescale(-3), Decimal{"3.211"});
}

TEST_F(TestChartDBFunctions, ComputeBoxsizeUsingMinMaxDataFromDB)  // NOLINT
{
    // we should get the same result as we do from tiingo, I expect

    PF_DB::DB_Params db_params{
        .user_name_ = "data_updater_pg", .db_name_ = "finance", .stock_db_data_source_ = "new_stock_data.current_data"};
    PF_DB the_db{db_params};

    Decimal close_range;

    std::string query =
        "select (max(split_adj_close) - min(split_adj_close)) as range from new_stock_data.current_data where date "
        "BETWEEN '2020-01-01' and '2023-04-01' "
        "and symbol = 'AAPL' ; ";

    // std::cout << query << std::endl;

    auto Row2Range = [](const auto& r) { return Decimal{r[0].template as<const char*>()}; };
    try
    {
        close_range = the_db.RunSQLQueryUsingRows<Decimal>(query, Row2Range)[0];
        // atr = ComputeATR("AAPL", price_data, history_size);
    }
    catch (const std::exception& e)
    {
        std::cout << "Unable to comput close range from DB for 'AAPL' because: " << e.what() << std::endl;
    }

    // results won't be exactly equal due to small differences in least
    // significant decimal digits of prices between tiingo and EODData
    EXPECT_EQ(close_range.rescale(-3), Decimal{"125.918"});
}

class PlotChartsWithChartDirector : public Test
{
};

TEST_F(PlotChartsWithChartDirector, Plot10X1Chart)  // NOLINT
{
    if (fs::exists("/tmp/candlestick1.svg"))
    {
        fs::remove("/tmp/candlestick1.svg");
    }
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 1);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    std::cout << chart << std::endl;

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 9);

    EXPECT_EQ(chart[5].GetTop(), 1120);
    EXPECT_EQ(chart[5].GetBottom(), 1110);
    EXPECT_EQ(chart[7].GetHadReversal(), true);

    //    std::cout << chart << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick1.svg", {}, "no");

    ASSERT_TRUE(fs::exists("/tmp/candlestick1.svg"));
}
TEST_F(PlotChartsWithChartDirector, Plot10X1ChartWithPrices)  // NOLINT
{
    if (fs::exists("/tmp/candlestick1_a.svg"))
    {
        fs::remove("/tmp/candlestick1_a.svg");
    }
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 1);
    const auto price_data_for_graphic = chart.LoadDataCollectPricesAndSignals(&prices, "%Y-%m-%d", ",");

    std::cout << chart << std::endl;

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 9);

    EXPECT_EQ(chart[5].GetTop(), 1120);
    EXPECT_EQ(chart[5].GetBottom(), 1110);
    EXPECT_EQ(chart[7].GetHadReversal(), true);

    //    std::cout << chart << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick1.svg_a", price_data_for_graphic, "no");

    ASSERT_TRUE(fs::exists("/tmp/candlestick1.svg_a"));
}
TEST_F(PlotChartsWithChartDirector, Plot10X2Chart)  // NOLINT
{
    if (fs::exists("/tmp/candlestick.svg"))
    {
        fs::remove("/tmp/candlestick.svg");
    }
    const std::string data =
        "1100 1105 1110 1112 1118 1120 1136 1121 1129 1120 1139 1121 1129 1138 1113 1139 1123 1128 1136 1111 1095 1102 "
        "1108 1092 1129 "
        "1122 1133 1125 1139 1105 1132 1122 1131 1127 1138 1111 1122 1111 1128 1115 1117 1120 1119 1132 1133 1147 1131 "
        "1159 1136 1127";

    std::string test_data =
        MakeSimpleTestData(data, std::chrono::year_month_day{2015y / std::chrono::March / std::chrono::Monday[1]}, " ");

    std::istringstream prices{test_data};

    PF_Chart chart("GOOG", 10, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Down);
    EXPECT_EQ(chart.size(), 6);

    EXPECT_EQ(chart[5].GetTop(), 1140);
    EXPECT_EQ(chart[5].GetBottom(), 1130);
    EXPECT_EQ(chart[5].GetHadReversal(), false);

    //    std::cout << chart << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick.svg", {}, "no");

    ASSERT_TRUE(fs::exists("/tmp/candlestick.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalData)  // NOLINT
{
    if (fs::exists("/tmp/candlestick2.svg"))
    {
        fs::remove("/tmp/candlestick2.svg");
    }
    const fs::path file_name{"./test_files/AAPL_close.dat"};

    std::ifstream prices{file_name};

    PF_Chart chart("AAPL", 2, 2);
    chart.LoadData(&prices, "%Y-%m-%d", ",");

    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_Up);
    EXPECT_EQ(chart.size(), 47);

    EXPECT_EQ(chart[47].GetTop(), 148);
    EXPECT_EQ(chart[47].GetBottom(), 146);

    //    std::cout << chart << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick2.svg", {}, "no");

    ASSERT_TRUE(fs::exists("/tmp/candlestick2.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalDataUsingComputedATR)  // NOLINT
{
    if (fs::exists("/tmp/candlestick3.svg"))
    {
        fs::remove("/tmp/candlestick3.svg");
    }
    const fs::path file_name{"./test_files/AAPL_short.json"};

    const std::string hist = LoadDataFileForUse(file_name);
    //    std::cout << "history length: " << hist.size() << '\n';

    const std::regex source{R"***("(open|high|low|close|adjOpen|adjHigh|adjLow|adjClose)":\s*([0-9]*\.[0-9]*))***"};
    const std::string dest{R"***("$1":"$2")***"};
    auto result1 = std::regex_replace(hist, source, dest);
    //    std::cout << "result length: " << result1.size() << '\n';
    // std::cout.write(result1.data(), 900);
    //    std::cout << " <== data\n";

    JSONCPP_STRING err;
    Json::Value history;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(result1.data(), result1.data() + result1.size(), &history, &err))
    {
        throw std::runtime_error("Problem parsing test data file: "s + err);
    }
    //    std::cout << "history length: " << history.size() << '\n';

    auto converted_history = ConvertJSONPriceHistory("AAPL", history, history.size(), UseAdjusted::e_Yes);
    auto atr = ComputeATR("AAPL", converted_history, history.size() - 1);

    //    std::cout << "ATR: " << atr << '\n';
    atr = atr.rescale(-2);
    //    std::cout << "ATR: " << atr << '\n';

    // // compute box size as a percent of ATR, eg. 0.1
    //
    // Decimal box_size = atr * 0.1;
    //
    // std::cout << "box size: " << box_size << '\n';
    // box_size.Rescale(-5);
    // std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", atr, 3, 0, BoxScale::e_Linear);

    rng::for_each(*const_cast<const Json::Value*>(&history) | vws::reverse | vws::take(history.size() - 1),
                  [&chart](const auto& e)
                  {
                      //            std::cout << "processing: " << e << '\n';
                      Decimal val{e["adjClose"].asString()};
                      //            std::cout << "val: " << val << '\n';
                      std::string dte{e["date"].asString()};
                      std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
                      auto the_date = StringToUTCTimePoint("%Y-%m-%d", date);
                      chart.AddValue(val, the_date);
                  });

    //    PF_Chart chart("AAPL", 2, 2);
    //    chart.LoadData(&prices, "%Y-%m-%d", ',');

    //    EXPECT_EQ(chart.GetCurrentDirection(), PF_Column::Direction::e_down);
    //    EXPECT_EQ(chart.size(), 62);
    //
    //    EXPECT_EQ(chart[61].GetTop(), 146);
    //    EXPECT_EQ(chart[61].GetBottom(), 144);

    //    std::cout << chart << '\n';
    //
    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick3.svg", {}, "no");

    ASSERT_TRUE(fs::exists("/tmp/candlestick3.svg"));
}

TEST_F(PlotChartsWithChartDirector, ProcessFileWithFractionalDataUsingBothArithmeticAndPercent)  // NOLINT
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
    //    std::cout << "history length: " << hist.size() << '\n';

    const std::regex source{R"***("(open|high|low|close|adjOpen|adjHigh|adjLow|adjClose)":([0-9]*\.[0-9]*))***"};
    const std::string dest{R"***("$1":"$2")***"};
    auto result1 = std::regex_replace(hist, source, dest);

    JSONCPP_STRING err;
    Json::Value history;

    Json::CharReaderBuilder builder;
    const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(result1.data(), result1.data() + result1.size(), &history, &err))
    {
        throw std::runtime_error("Problem parsing test data file: "s + err);
    }
    //    std::cout << "history length: " << history.size() << '\n';

    Decimal box_size{".1"};

    // PF_Chart chart("YHOO", box_size, 3, BoxType::e_fractional);
    PF_Chart chart("YHOO", 1, 3, box_size, BoxScale::e_Linear, 150);

    rng::for_each(*const_cast<const Json::Value*>(&history) | vws::reverse | vws::take(history.size() - 1),
                  [&chart](const auto& e)
                  {
                      Decimal val{e["adjClose"].asString()};
                      std::string dte{e["date"].asString()};
                      std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
                      auto the_date = StringToUTCTimePoint("%Y-%m-%d", date);
                      chart.AddValue(val, the_date);
                  });

    //    std::cout << "# of cols: " << chart.size() << '\n';

    //    std::cout << chart << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/candlestick3.svg", {}, "no");

    EXPECT_TRUE(fs::exists("/tmp/candlestick3.svg"));

    PF_Chart chart_percent("YHOO", box_size, 3, 0, BoxScale::e_Percent);

    rng::for_each(*const_cast<const Json::Value*>(&history) | vws::reverse | vws::take(history.size() - 1),
                  [&chart_percent](const auto& e)
                  {
                      Decimal val{e["adjClose"].asString()};
                      std::string dte{e["date"].asString()};
                      std::string_view date{dte.begin(), dte.begin() + dte.find('T')};
                      auto the_date = StringToUTCTimePoint("%Y-%m-%d", date);
                      chart_percent.AddValue(val, the_date);
                  });

    //    std::cout << chart_percent << '\n';
    //    std::cout << "# of cols: " << chart_percent.size() << '\n';

    ConstructCDChartGraphicAndWriteToFile(chart_percent, "/tmp/candlestick4.svg", {}, "no");

    EXPECT_TRUE(fs::exists("/tmp/candlestick4.svg"));
}

TEST_F(PlotChartsWithChartDirector, LoadDataFromLiveDBUseMinMaxForLinearChart)  // NOLINT
{
    if (fs::exists("/tmp/linear14.svg"))
    {
        fs::remove("/tmp/linear14.svg");
    }
    if (fs::exists("/tmp/percent14.svg"))
    {
        fs::remove("/tmp/percent14.svg");
    }

    PF_DB::DB_Params db_params{.user_name_ = "data_updater_pg",
                               .db_name_ = "finance",
                               .PF_db_mode_ = "live",
                               .stock_db_data_source_ = "new_stock_data.current_data"};
    PF_DB the_db{db_params};

    Decimal close_range;

    std::string query =
        "select (max(split_adj_close) - min(split_adj_close)) as range from new_stock_data.current_data where date "
        "BETWEEN '2017-01-01' and '2023-04-01' "
        "and symbol = 'AAPL' ; ";
    auto Row2Range = [](const auto& r) { return Decimal{r[0].template as<const char*>()}; };
    try
    {
        close_range = the_db.RunSQLQueryUsingRows<Decimal>(query, Row2Range)[0];
        // atr = ComputeATR("AAPL", price_data, history_size);
    }
    catch (const std::exception& e)
    {
        std::cout << "Unable to comput close range from DB for 'AAPL' because: " << e.what() << std::endl;
    }
    //    std::cout << "history length: " << history.size() << '\n';

    std::string get_symbol_prices_cmd = std::format(
        "SELECT date, {} FROM new_stock_data.current_data WHERE date BETWEEN '2017-01-01' and '2023-04-01' AND symbol "
        "= 'AAPL' ORDER BY "
        "date ASC",
        "split_adj_close");

    const auto* dt_format = "%F";

    std::istringstream time_stream;
    date::utc_time<std::chrono::utc_clock::duration> tp;

    // we know our database contains 'date's, but we need timepoints.
    // we'll handle that in the conversion routine below.

    auto Row2Closing = [dt_format, &time_stream, &tp](const auto& r)
    {
        time_stream.clear();
        time_stream.str(std::string{std::get<0>(r)});
        date::from_stream(time_stream, dt_format, tp);
        std::chrono::utc_time<std::chrono::utc_clock::duration> tp1{tp.time_since_epoch()};
        DateCloseRecord new_data{.date_ = tp1, .close_ = Decimal{std::get<1>(r)}};
        return new_data;
    };
    const auto closing_prices = the_db.RunSQLQueryUsingStream<DateCloseRecord, std::string_view, const char*>(
        get_symbol_prices_cmd, Row2Closing);

    Decimal box_size{".01"};

    // PF_Chart chart("YHOO", box_size, 3, BoxType::e_fractional);
    PF_Chart chart("AAPL", close_range, 2, box_size, BoxScale::e_Linear, 150);
    // std::print("Linar chart before data: {}\n", chart);

    for (const auto& [new_date, new_price] : closing_prices)
    {
        // std::cout << "new value: " << new_price << "\t" << new_date << std::endl;
        chart.AddValue(new_price, std::chrono::clock_cast<std::chrono::utc_clock>(new_date));
    }

    // std::print("Linear chart: {}\n", chart);

    ConstructCDChartGraphicAndWriteToFile(chart, "/tmp/linear14.svg", {}, "no");

    EXPECT_TRUE(fs::exists("/tmp/linear14.svg"));

    PF_Chart chart_percent("AAPL", close_range, 2, box_size, BoxScale::e_Percent, 150);

    for (const auto& [new_date, new_price] : closing_prices)
    {
        // std::cout << "new value: " << new_price << "\t" << new_date << std::endl;
        chart_percent.AddValue(new_price, std::chrono::clock_cast<std::chrono::utc_clock>(new_date));
    }
    ConstructCDChartGraphicAndWriteToFile(chart_percent, "/tmp/percent14.svg", {}, "no");

    // std::print("Percent chart: {}\n", chart_percent);

    EXPECT_TRUE(fs::exists("/tmp/percent14.svg"));
}

class TiingoATR : public Test
{
    std::string LoadApiKey(std::string file_name) const
    {
        if (!fs::exists(file_name))
        {
            throw std::runtime_error("Can't find key file.");
        }
        std::ifstream key_file(file_name);
        std::string result;
        key_file >> result;
        return result;
    }

   public:
    const std::string api_key_ = LoadApiKey("./tiingo_key.dat");
};

TEST_F(TiingoATR, RetrievePreviousData)  // NOLINT
{
    std::chrono::year which_year = 2021y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key_};

    auto history = history_getter.GetMostRecentTickerData(
        "AAPL", std::chrono::year_month_day{2021y / std::chrono::October / 7}, 14, UseAdjusted::e_No, &holidays);

    EXPECT_EQ(history.size(), 14);
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0].date_),
              std::chrono::year_month_day{2021y / std::chrono::October / 7});
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[13].date_),
              std::chrono::year_month_day{2021y / std::chrono::September / 20});
}

TEST_F(TiingoATR, RetrievePreviousCloseAndCurrentOpen)  // NOLINT
{
    // for streaming, we want to retrieve the previous day's close and, if the markets
    // are already open, the day's open.  We do this to capture 'gaps' and to set
    // the direction at little sooner.

    auto today = std::chrono::year_month_day{floor<std::chrono::days>(std::chrono::system_clock::now())};
    std::chrono::year which_year = today.year();
    auto holidays = MakeHolidayList(which_year);
    rng::copy(MakeHolidayList(--which_year), std::back_inserter(holidays));

    auto current_local_time = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                         floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    auto market_status =
        GetUS_MarketStatus(std::string_view{std::chrono::current_zone()->name()}, current_local_time.get_local_time());

    if (market_status != US_MarketStatus::e_NotOpenYet && market_status != US_MarketStatus::e_OpenForTrading)
    {
        //        std::cout << "Market not open for trading now so we can't stream quotes.\n";
        return;
    }

    Tiingo history_getter{"api.tiingo.com", "443", "/iex", api_key_, std::vector<std::string>{"spy", "uso", "rsp"}};

    if (market_status == US_MarketStatus::e_NotOpenYet)
    {
        auto history = history_getter.GetMostRecentTickerData("AAPL", today, 2, UseAdjusted::e_No, &holidays);

        EXPECT_EQ(history.size(), 1);
        auto business_days = ConstructeBusinessDayRange(today, 2, UpOrDown::e_Down, &holidays);
        EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0].date_), business_days.second);
    }
    if (market_status == US_MarketStatus::e_OpenForTrading)
    {
        auto history = history_getter.GetTopOfBookAndLastClose();
        for (const auto& e : history)
        {
            const std::string ticker = e["ticker"].asString();
            const std::string tstmp = e["timestamp"].asString();
            const auto time_stamp = StringToUTCTimePoint("%FT%T%z", tstmp);

            //            std::cout << "ticker: " << ticker << " tstmp: " << tstmp << " time stamp: " <<
            //            std::format("{}", time_stamp) <<
            //            '\n';
        }
        //        std::cout << history << '\n';
        EXPECT_EQ(history.size(), 3);
    }
}

TEST_F(TiingoATR, RetrievePreviousDataThenComputeAverageTrueRange)  // NOLINT
{
    std::chrono::year which_year = 2021y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key_};

    auto history = history_getter.GetMostRecentTickerData(
        "AAPL", std::chrono::year_month_day{2021y / std::chrono::October / 7}, 15, UseAdjusted::e_No, &holidays);
    //    // rng::for_each(history, [](const auto& e){ std::print("{}\n", e); });

    EXPECT_EQ(history.size(), 15);
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[0].date_),
              std::chrono::year_month_day{2021y / std::chrono::October / 7});
    EXPECT_EQ(StringToDateYMD("%Y-%m-%d", history[4].date_),
              std::chrono::year_month_day{2021y / std::chrono::October / 1});

    // auto atr = ComputeATRUsingJSON("AAPL", history, 4);
    auto atr = ComputeATR("AAPL", history, 4);
    // std::print("ATR: {}\n", atr);
    ASSERT_TRUE(atr == Decimal{"3.369"});
}

TEST_F(TiingoATR, ComputeATRThenBoxSizeBasedOn20DataPoints)  // NOLINT
{
    std::chrono::year which_year = 2021y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key_};

    constexpr int history_size = 20;
    const auto history =
        history_getter.GetMostRecentTickerData("AAPL", std::chrono::year_month_day{2021y / std::chrono::October / 7},
                                               history_size + 1, UseAdjusted::e_No, &holidays);
    // rng::for_each(history, [](const auto& h) { std::cout << std::format("{}\n", h); });

    //     auto atr = ComputeATRUsingJSON("AAPL", history, 4);
    // //    std::cout << "ATR: " << atr << '\n';
    //     EXPECT_TRUE(atr == Decimal{"3.36875"});

    // recompute using all the data for rest of test

    auto atr = ComputeATR("AAPL", history, history_size);
    // std::cout << "ATR using 20 days: " << atr << '\n';
    EXPECT_EQ(atr.rescale(-3), Decimal{"3.211"});

    // next, I need to compute my average closing price over the interval
    // but excluding the 'extra' value included for computing the ATR

    auto bkwd_data = history | vws::reverse | vws::take(history_size) |
                     vws::transform([](const StockDataRecord& e) { /* std::cout << std::format("{}\n", e); */
                                                                   return e.close_;
                     });
    Decimal sum = std::accumulate(bkwd_data.begin(), bkwd_data.end(), Decimal{0}, std::plus<>());
    // std::cout << "sum: " << sum << '\n';
    Decimal box_size = atr / (sum / history_size);

    std::cout << "atr: " << atr << '\n';
    box_size = box_size.rescale(-5);
    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", atr, 2, box_size, BoxScale::e_Linear);

    // ticker data retrieved above is in descending order by date, so let's read it backwards
    // but, there are no reverse iterator provided so let's see if ranges will come to the rescue

    //    auto backwards = history | vws::reverse;

    rng::for_each(history | vws::reverse | vws::take(history_size),
                  [&chart](const auto& e)
                  {
                      // std::string_view date{e.date_.data(), e.date_.data() + e.date_.find('T')};
                      auto the_date = StringToUTCTimePoint("%Y-%m-%d", e.date_);
                      chart.AddValue(e.close_, the_date);
                  });

    //   std::cout << chart << '\n';
}

TEST_F(TiingoATR, ComputeATRThenBoxSizeBasedOn20DataPointsUsePercentValues)  // NOLINT
{
    std::chrono::year which_year = 2021y;
    auto holidays = MakeHolidayList(which_year);

    Tiingo history_getter{"api.tiingo.com", "443", api_key_};

    constexpr int history_size = 20;
    const auto history =
        history_getter.GetMostRecentTickerData("AAPL", std::chrono::year_month_day{2021y / std::chrono::October / 7},
                                               history_size + 1, UseAdjusted::e_No, &holidays);
    //    // rng::for_each(history, [](const auto& e) { std::print("{}\n", e); });

    auto atr = ComputeATR("AAPL", history, 4);
    //    std::cout << "ATR: " << atr << '\n';
    EXPECT_EQ(atr, Decimal{"3.369"});

    // recompute using all the data for rest of test

    atr = ComputeATR("AAPL", history, history_size);

    // next, I need to compute my average closing price over the interval
    // but excluding the 'extra' value included for computing the ATR

    // Decimal sum = rng::accumulate(history | vws::reverse | vws::take(history_size),
    //         Decimal{}, std::plus<>(),
    //         [](const StockDataRecord& e) { return e.close_; });
    //
    // Decimal box_size = atr / (sum / history_size);

    //    std::cout << "box size: " << box_size << '\n';
    // box_size.Rescale(-5);
    //    std::cout << "rescaled box size: " << box_size << '\n';

    PF_Chart chart("AAPL", atr, 2, Decimal(".01"), BoxScale::e_Percent);

    // ticker data retrieved above is in descending order by date, so let's read it backwards
    // but, there are no reverse iterator provided so let's see if ranges will come to the rescue

    //    auto backwards = history | vws::reverse;

    rng::for_each(history | vws::reverse | vws::take(history_size),
                  [&chart](const auto& e)
                  {
                      auto the_date = StringToUTCTimePoint("%Y-%m-%d", e.date_);
                      auto status = chart.AddValue(e.close_, the_date);
                      //            std::print("value: {} status: {}\n", e.close_, status);
                  });

    std::cout << std::format("AAPL chart: {}\n", chart);

    // TODO(dpriedel): I've lost the plot on this test and don't remember what else I
    // wanted to do. So, I'll leave this output so I'll be reminded when I run these tests.

    //    rng::for_each(history | vws::reverse , [](const auto& e) { std::cout << std::format("date: {} close: {}
    //    adjusted close: {} delta:
    //    {} \n",
    //                e["date"].asString(), e["close"].asString(), e["split_adj_close"].asString(), 0); });
}

class WebSocketSynchronous : public Test
{
    std::string LoadApiKey(std::string file_name)
    {
        if (!fs::exists(file_name))
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

TEST_F(WebSocketSynchronous, ConnectAndDisconnect)  // NOLINT
{
    auto current_local_time = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                         floor<std::chrono::seconds>(std::chrono::system_clock::now()));
    auto can_we_stream = GetUS_MarketStatus(std::string_view{std::chrono::current_zone()->name()},
                                            current_local_time.get_local_time()) == US_MarketStatus::e_OpenForTrading;

    if (!can_we_stream)
    {
        //        std::cout << "Market not open for trading now so we can't stream quotes.\n";
        return;
    }

    Tiingo quotes{"api.tiingo.com", "443", "/iex", api_key, std::vector<std::string>{"spy", "uso", "rsp"}};
    quotes.Connect();
    bool time_to_stop = false;

    std::mutex data_mutex;
    std::queue<std::string> streamed_data;

    auto streaming_task =
        std::async(std::launch::async, &Tiingo::StreamData, &quotes, &time_to_stop, &data_mutex, &streamed_data);

    std::this_thread::sleep_for(10s);
    time_to_stop = true;
    streaming_task.get();
    //    ASSERT_EXIT((the_task.get()),::testing::KilledBySignal(SIGINT),".*");
    quotes.Disconnect();

    //    for (const auto & value: streamed_data)
    //    {
    //        std::cout << value << '\n';
    //    }
    ASSERT_TRUE(!streamed_data.empty());  // we need an actual test here
}

// NOLINTEND(*-magic-numbers)

//===  FUNCTION  ======================================================================
//        Name:  InitLogging
// Description:
//=====================================================================================

void InitLogging()
{
    spdlog::set_level(spdlog::level::err);
    //    spdlog::get()->set_filter
    //    (
    //        logging::trivial::severity >= logging::trivial::trace
    //    );
} /* -----  end of function InitLogging  ----- */

int main(int argc, char** argv)
{
    decimal::context_template = decimal::IEEEContext(decimal::DECIMAL64);
    decimal::context_template.round(decimal::ROUND_HALF_EVEN);
    decimal::context = decimal::context_template;

    InitLogging();

    // py::scoped_interpreter guard{false};  // start the interpreter and keep it alive
    //
    // py::print("Hello, World!");  // use the Python API
    //
    // py::exec(R"(
    //     import PF_DrawChart_prices as PF_DrawChart
    //     )");

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
