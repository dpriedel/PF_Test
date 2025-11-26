// =====================================================================================
//
//       Filename:  EndToEndTest.cpp
//
//    Description:  Driver program for end-to-end tests
//
//        Version:  1.0
//        Created:  2021-11-20 09:07 AM
//       Revision:  none
//       Compiler:  g++
//
//         Author:  David P. Riedel (dpr), driedel@cox.net
//        License:  GNU General Public License v3
//        Company:
//
// =====================================================================================

// =====================================================================================
//        Class:
//  Description:
// =====================================================================================

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <future>
#include <ranges>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "PF_CollectDataApp.h"
#include "utilities.h"

namespace rng = std::ranges;
namespace vws = std::ranges::views;

#include <gmock/gmock.h>
#include <gtest/gtest.h>

// #include <pqxx/pqxx>
// #include <pqxx/transaction.hxx>

using namespace std::literals::chrono_literals;
using namespace std::string_literals;

namespace fs = std::filesystem;

const fs::path APPL_EOD_CSV{"./test_files/AAPL_close.dat"};
const fs::path SPY_EOD_CSV{"./test_files/SPY.csv"};

using namespace testing;

std::optional<int> FindColumnIndex(std::string_view header, std::string_view column_name, std::string_view delim)
{
    auto fields = rng_split_string<std::string_view>(header, delim);
    auto do_compare([&column_name](const auto &field_name) {
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
    }
    return {};

} // -----  end of method PF_CollectDataApp::FindColumnIndex  -----

class ProgramOptions : public Test
{
};

TEST_F(ProgramOptions, TestMixAndMatchOptions) // NOLINT
{
    // setenv("PF_COLLECT_DATA_CONFIG_DIR", "/home/dpriedel/.config/PF_CollectData", true);

    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_eod.json");
    }
    if (fs::exists("/tmp/test_charts/SPY_10X1_linear_eod.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X1_linear_eod.json");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.
    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "-s", "SpY",
        "-s", "aapL",
        "--symbol", "IWr",
        "--new-data-source", "file",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--boxsize", "1",
        "--reversal", "3",
        "--reversal", "1",
        "--log-path", "/tmp/PF_Collect/test01.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X1_linear_eod.json"));
}

TEST_F(ProgramOptions, DISABLED_TestProblemOptions) // NOLINT
{
    //	NOTE: disabled because now I am capturing error internally which would
    //	have generated the exception this is testing for.
    //
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "-s", "qqqq",
        "-s", "spy",
        "--new-data-source", "streaming",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--use-ATR",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test02.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            // 'qqqq' is not a valid symbol so there will be zero prices returned
            // for ATR calculation. Hence the 'throw'.
            ASSERT_THROW(myApp.Run(), std::invalid_argument);
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
}

TEST_F(ProgramOptions, TestMinMaxOptions) // NOLINT
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "-s", "qqqq",
        "-s", "spy",
        "--new-data-source", "streaming",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--use-ATR",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test03.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        // both use-ATR and use-MinMax specified so this is an error
        EXPECT_FALSE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    // clang-format off
	std::vector<std::string> tokens2{"the_program",
        "-s", "qqqq",
        "-s", "spy",
        "--new-data-source", "streaming",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test04.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens2);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        // use-MinMax is specified but data source is 'streaming' so this is an error
        EXPECT_FALSE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    // clang-format off
	std::vector<std::string> tokens3{"the_program",
        "-s", "qqqq",
        "-s", "spy",
        "--new-data-source", "database",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        // "--new-data-dir", "./test_files",
        // "--source-format", "csv",
        "--mode", "update",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test05.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens3);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        // use-MinMax is specified but mode is 'update' so this is an error
        EXPECT_FALSE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    // clang-format off
	std::vector<std::string> tokens4{"the_program",
        "-s", "qqqq",
        "-s", "spy",
        "--new-data-source", "database",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--begin-date", "2017-01-01",
        // "--new-data-dir", "./test_files",
        // "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test06.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens4);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        EXPECT_TRUE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
}

TEST_F(ProgramOptions, TestExchangesList) // NOLINT
{
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--new-data-source", "database",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2017-01-01",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "percent",
        "--price-fld-name", "split_adj_close",
        "--destination", "database",
        "--graphics-format", "csv",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--exchange-list", "NYSE,nasdaq",
        "--log-path", "/tmp/PF_Collect/test07.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        // both use-ATR and use-MinMax specified so this is an error
        EXPECT_TRUE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    // clang-format off
	std::vector<std::string> tokens2{"the_program",
        "--symbol-list", "ALL",
        "--new-data-source", "database",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "percent",
        "--price-fld-name", "split_adj_close",
        "--destination", "database",
        "--graphics-format", "csv",
        "--use-MinMax",
        "--boxsize", ".1",
        "--boxsize", ".01",
        "--reversal", "1",
        "--reversal", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2017-01-01",
        "--exchange-list", "NYSE,nasdax",
        "--log-path", "/tmp/PF_Collect/test08.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens2);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        // both use-ATR and use-MinMax specified so this is an error
        EXPECT_FALSE(startup_OK);
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
}

class SingleFileEndToEnd : public Test
{
};

TEST_F(SingleFileEndToEnd, VerifyCanLoadCSVDataAndSaveToChartFile) // NOLINT
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_eod.json");
    }
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--new-data-source", "file",
        "--new-data-dir", "./test_files3",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test09.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"));
}

TEST_F(SingleFileEndToEnd, VerifyCanConstructChartFileFromPieces) // NOLINT
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_eod.json");
    }
    if (fs::exists("/tmp/test_charts2/SPY_10X3_linear_eod.json"))
    {
        fs::remove("/tmp/test_charts2/SPY_10X3_linear_eod.json");
    }
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // the first test run constructs the data file all at once

    PF_Chart whole_chart;

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--new-data-source", "file",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test10.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            whole_chart = myApp.GetCharts()[0].second;
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_eod.json"));

    // now construct the data file from 2 input files which, together, contain the same
    // data as the 1 file used above.

    PF_Chart half_chart;

    // clang-format off
	std::vector<std::string> tokens2{"the_program",
        "--symbol", "SPY",
        "--new-data-source", "file",
        "--new-data-dir", "./test_files2",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts2",
        "--boxsize", "10",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test11.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens2);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            half_chart = myApp.GetCharts()[0].second;
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear_eod.json"));

    // now continue constructing the data file from 2 input files which, together, contain the same
    // data as the 1 file used above.

    PF_Chart franken_chart;

    // clang-format off
	std::vector<std::string> tokens3{"the_program",
        "--symbol", "SPY",
        "--new-data-source", "file",
        "--new-data-dir", "./test_files3",
        "--chart-data-dir", "/tmp/test_charts2",
        "--source-format", "csv",
        "--mode", "update",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts2",
        "--boxsize", "10",
        "--reversal", "3",
        "--log-path", "/tmp/PF_Collect/test12.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens3);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            franken_chart = myApp.GetCharts()[0].second;
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    std::cout << "\n\n whole chart:\n" << whole_chart;
    std::cout << "\n\n half chart:\n" << half_chart;
    std::cout << "\n\n franken chart:\n" << franken_chart << '\n';
    ;

    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear_eod.json"));
    ASSERT_TRUE(whole_chart == franken_chart);
}

class LoadAndUpdate : public Test
{
};

TEST_F(LoadAndUpdate, VerifyUpdateWorksWhenNoPreviousChartData) // NOLINT
{
    if (fs::exists("/tmp/test_charts_updates"))
    {
        fs::remove_all("/tmp/test_charts_updates");
    }
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "-s", "ADIV", "-s", "AIA", "-s", "AWF", "-s", "CBRL",
        "--new-data-source", "file",
        "--quote-host", "eodhd.com",
        "--quote-data-source", "Eodhd",
        "--quote-api-key", "Eodhd_key.dat",
        "--new-data-dir", "./test_files_update_EOD",
        "--source-format", "csv",
        "--mode", "update",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "adjClose",
        "--destination", "file",
        "--chart-data-dir", "./test_files_update_charts",
        "--output-chart-dir", "/tmp/test_charts_updates",
        "--use-ATR",
        "--boxsize", ".1",
        "--reversal", "3", "--reversal", "1",
        "--log-path", "/tmp/PF_Collect/test13.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    EXPECT_TRUE(fs::exists("/tmp/test_charts_updates/CBRL_0.1X3_linear_eod.json"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts_updates/CBRL_0.1X3_linear_eod.svg"));
}

class Database : public Test
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

        int count = trxn.query_value<int>("SELECT count(*) FROM test_point_and_figure.pf_charts");
        trxn.commit();
        return count;
    }

    bool CheckLastUpdateDateEqualsLastCheckDate()
    {
        std::string date_query =
            " SELECT a = b FROM (SELECT (to_timestamp( (chart_data -> 'current_column' -> 'last_entry')::BIGINT / "
            "1000000000) AT TIME ZONE 'utc')::DATE FROM test_point_and_figure.pf_charts WHERE symbol = 'CECO' LIMIT 1) "
            "AS b, ( SELECT (max(last_change_date) AT TIME ZONE 'utc')::DATE FROM test_point_and_figure.pf_charts "
            "WHERE symbol = 'CECO') AS a; ";
        // std::string date_query =
        //     " SELECT a = b FROM ( SELECT ( SELECT ( to_timestamp( (chart_data -> 'current_column' -> "
        //     "'last_entry')::BIGINT / 1000000000) AT TIME ZONE 'utc')::DATE FROM test_point_and_figure.pf_charts WHERE
        //     " "symbol = 'CECO' LIMIT 1) AS b, ( SELECT (max(last_checked_date) AT TIME ZONE 'utc')::DATE FROM "
        //     "test_point_and_figure.pf_charts WHERE symbol = 'CECO') AS a); ";

        pqxx::connection c{"dbname=finance user=data_updater_pg"};
        pqxx::nontransaction trxn{c};

        bool dates_are_equal = trxn.query_value<bool>(date_query);
        trxn.commit();
        return dates_are_equal;
    }
};

TEST_F(Database, LoadDataFromDB) // NOLINT
{
    if (fs::exists("/tmp/test_charts"))
    {
        fs::remove_all("/tmp/test_charts");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",      // want to use SP500 indicator but need to do more setup first
        "--symbol", "AAPL",
        "--symbol-list", "IWR,iwm,t",
        "--new-data-source", "database",
        "--mode", "load",
        "--scale", "linear",
        "--price-fld-name", "split_adj_close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", ".1",
        "--boxsize", "1",
        "--reversal", "1",
        "-r", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2017-01-01",
        "--use-ATR",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test14.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_1X3_linear_eod.json"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_0.1X1_linear_eod.json"));
}

TEST_F(Database, DISABLED_BulkLoadDataFromDB) // NOLINT
{
    if (fs::exists("/tmp/test_charts3"))
    {
        fs::remove_all("/tmp/test_charts3");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol-list", "ALL",
        // "-s", "ACY",
        "--new-data-source", "database",
        "--mode", "load",
        "--scale", "percent",
        "--price-fld-name", "split_adj_close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts3",
        "--graphics-format", "svg",
        // "--boxsize", ".1",
        "--boxsize", ".01",
        "--boxsize", ".001",
        "--reversal", "1",
        "-r", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2022-06-01",
        "--use-MinMax",
        "--exchange-list", "amex,bats",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test15.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts3/SPY_0.01%X1_percent_eod.csv"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts3/SPY_0.001%X1_percent_eod.json"));
}

TEST_F(Database, UpdateUsingDataFromDB) // NOLINT
{
    if (fs::exists("/tmp/test_charts2"))
    {
        fs::remove_all("/tmp/test_charts2");
    }

    fs::create_directories("/tmp/test_charts2");

    // construct a chart using some test data and save it.
    fs::path csv_file_name{SPY_EOD_CSV};
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
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record) {
                      const auto fields = split_string<std::string_view>(record, ",");
                      new_chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
                  });
    std::cout << "new chart at after loading initial data: \n\n" << new_chart << "\n\n";

    fs::path chart_file_path = fs::path{"/tmp/test_charts2"} / (new_chart.MakeChartFileName("eod", "json"));
    std::ofstream new_file{chart_file_path, std::ios::out | std::ios::binary};
    BOOST_ASSERT_MSG(new_file.is_open(),
                     std::format("Unable to open file: {} to write updated data.", chart_file_path).c_str());
    new_chart.ConvertChartToJsonAndWriteToStream(new_file);
    new_file.close();

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",      // want to use SP500 indicator but need to do more setup first
        "--symbol", "AAPL",      // want to use SP500 indicator but need to do more setup first
        "--symbol", "GOOG",      // want to use SP500 indicator but need to do more setup first
        "--new-data-source", "database",
        "--mode", "update",
        "--scale", "linear",
        "--price-fld-name", "split_adj_close",
        "--destination", "file",
        "--chart-data-source", "file",
        "--chart-data-dir", "/tmp/test_charts2",
        "--output-chart-dir", "/tmp/test_charts2",
        "--boxsize", "10",
        "--reversal", "1",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2021-11-24",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test16.log"
	};
    // clang-format on

    PF_Chart updated_chart;

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            updated_chart = myApp.GetCharts()[0].second;
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    std::cout << "updated chart at after loading initial data: \n\n" << updated_chart << "\n\n";

    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X1_linear_eod.json"));
    ASSERT_NE(new_chart, updated_chart);
}

TEST_F(Database, UpdateDatainDBUsingNewDataFromDB) // NOLINT
{
    if (fs::exists("/tmp/test_charts9"))
    {
        fs::remove_all("/tmp/test_charts9");
    }
    // construct a chart using some test data and save it.
    fs::path csv_file_name{SPY_EOD_CSV};
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
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record) {
                      const auto fields = split_string<std::string_view>(record, ",");
                      new_chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
                  });
    std::cout << "new chart at after loading initial data: \n\n" << new_chart << "\n\n";

    PF_DB::DB_Params db_info{.user_name_ = "data_updater_pg", .db_name_ = "finance", .PF_db_mode_ = "test"};
    PF_DB pf_db(db_info);

    new_chart.StoreChartInChartsDB(pf_db, "eod");

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",      // want to use SP500 indicator but need to do more setup first
        "--new-data-source", "database",
        "--mode", "update",
        "--scale", "linear",
        "--price-fld-name", "split_adj_close",
        "--destination", "database",
        // "--destination", "file",
        "--chart-data-source", "database",
        "--output-chart-dir", "/tmp/test_charts9",
        "--output-graph-dir", "/tmp/test_charts9",
        "--graphics-format", "svg",
        "--boxsize", "10",
        "--boxsize", "5",
        "--reversal", "1",
        "--reversal", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2021-11-24",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test17.log"
	};
    // clang-format on

    PF_Chart updated_chart;

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            updated_chart = myApp.GetCharts()[0].second;
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    std::cout << "updated chart at after loading initial data: \n\n" << updated_chart << "\n\n";

    EXPECT_EQ(CountRows(), 4);
    ASSERT_NE(new_chart, updated_chart);
}

TEST_F(Database, DISABLED_BulkLoadDataFromDBAndStoreChartsInDB) // NOLINT
{
    if (fs::exists("/tmp/test_charts3"))
    {
        fs::remove_all("/tmp/test_charts3");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol-list", "ALL",
        // "-s", "ACY",
        "--new-data-source", "database",
        "--mode", "load",
        "--scale", "percent",
        "--price-fld-name", "split_adj_close",
        "--destination", "database",
        "--output-graph-dir", "/tmp/test_charts3",
        "--graphics-format", "csv",
        "--boxsize", ".1",
        "--boxsize", ".01",
        // "--boxsize", ".001",
        "--reversal", "1",
        "-r", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2022-01-01",
        "--use-ATR",
        "--exchange", "NYSE",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test18.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts3/SPY_0.01%X1_percent_eod.csv"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts3/SPY_0.001%X1_percent_eod.json"));
}

TEST_F(Database, LoadDataFromDBWithMinMaxAndStoreChartsInDirectory) // NOLINT
{
    if (fs::exists("/tmp/test_charts13"))
    {
        fs::remove_all("/tmp/test_charts13");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol-list", "AAPL,GOOG,IWM,IWR,spy,qqq,A,rsp",
        // "--symbol-list", "A",
        // "-s", "ACY",
        "--new-data-source", "database",
        "--mode", "load",
        "--scale", "linear",
        "--price-fld-name", "split_adj_close",
        // "--destination", "database",
        "--destination", "file",
        "--output-graph-dir", "/tmp/test_charts13",
        "--output-chart-dir", "/tmp/test_charts13",
        "--graphics-format", "svg",
        // "--boxsize", ".1",
        "--boxsize", ".01",
        "--boxsize", ".005",
        "--reversal", "1",
        "-r", "3",
        "--db-mode", "test",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2017-01-01",
        "--use-MinMax",
        // "--exchange", "NYSE",
        // "-l", "debug",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test19.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_TRUE(fs::exists("/tmp/test_charts13/SPY_0.01X1_linear_eod.svg"));
}

TEST_F(Database, LoadDataFromDBAndStoreInDBVerifyLastChangeDatesMatch) // NOLINT
{
    if (fs::exists("/tmp/test_charts13a"))
    {
        fs::remove_all("/tmp/test_charts13a");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "CECO",
        // "--symbol-list", "A",
        // "-s", "ACY",
        "--new-data-source", "database",
        "--chart-data-source", "database",
        "--mode", "load",
        "--scale", "linear",
        "--price-fld-name", "split_adj_close",
        "--destination", "database",
        // "--destination", "file",
        "--output-graph-dir", "/tmp/test_charts13a",
        "--output-chart-dir", "/tmp/test_charts13a",
        "--graphics-format", "csv",
        // "--boxsize", ".1",
        "--boxsize", ".01",
        "--boxsize", ".005",
        "--reversal", "1",
        "-r", "2",
        "-r", "3",
        "--db-mode", "test",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2017-01-01",
        // "--end-date", "2025-07-09",
        "--use-MinMax",
        // "--exchange", "NYSE",
        // "-l", "debug",
        "--max-graphic-cols", "150",
        "--log-path", "/tmp/PF_Collect/test20.log"
	};
    // clang-format on

    PF_Chart updated_chart;

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    // std::cout << "updated chart at after loading initial data: \n\n" << updated_chart << "\n\n";

    ASSERT_TRUE(CheckLastUpdateDateEqualsLastCheckDate());
}

TEST_F(Database, DailyScan) // NOLINT
{
    // construct a chart using some test data and save it.
    fs::path csv_file_name{SPY_EOD_CSV};
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
                  [&new_chart, close_col = close_column.value(), date_col = date_column.value()](const auto record) {
                      const auto fields = split_string<std::string_view>(record, ",");
                      new_chart.AddValue(sv2dec(fields[close_col]), StringToUTCTimePoint("%Y-%m-%d", fields[date_col]));
                  });
    // std::cout << "new chart at after loading initial data: \n\n" << new_chart << "\n\n";

    PF_DB::DB_Params db_info{.user_name_ = "data_updater_pg", .db_name_ = "finance", .PF_db_mode_ = "test"};
    PF_DB pf_db(db_info);

    new_chart.StoreChartInChartsDB(pf_db, "eod");

    // save for comparison later

    PF_Chart saved_chart{new_chart};

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--mode", "daily-scan",
        "--exchange-list", "amex,nyse,nasdaq",
        "--price-fld-name", "split_adj_close",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--stock-db-data-source", "new_stock_data.current_data",
        "--begin-date", "2025-07-01",
        "--log-path", "/tmp/PF_Collect/test21.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }

    EXPECT_EQ(CountRows(), 1);

    // let's see what is in the DB

    auto updated_chart = PF_Chart::LoadChartFromChartsDB(pf_db, saved_chart.GetChartParams(), "eod");
    // std::cout << "updated chart at after after running daily scan: \n\n" << updated_chart << "\n\n";

    ASSERT_NE(saved_chart, updated_chart);
}

class StreamEodhdData : public Test
{
};

TEST_F(StreamEodhdData, VerifyConnectAndDisconnect) // NOLINT
{
    if (fs::exists("/tmp/test_charts_Eodhd"))
    {
        fs::remove_all("/tmp/test_charts_Eodhd");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--symbol", "AAPL",
        "--new-data-source", "streaming",
        "--quote-host", "eodhd.com",
        "--quote-data-source", "Eodhd",
        "--quote-api-key", "Eodhd_key.dat",
        "--streaming-host", "ws.eodhistoricaldata.com",
        "--streaming-data-source", "Eodhd",
        "--streaming-api-key", "Eodhd_key.dat",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts_Eodhd",
        "--boxsize", "0.1",
        "--boxsize", "0.05",
        "--reversal", "1",
        "-l", "debug",
        "--log-path", "/tmp/PF_Collect/test22.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                              floor<std::chrono::seconds>(std::chrono::system_clock::now()));
        auto then = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                               floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 15s);

        int counter = 0;
        auto timer = [&counter](const auto &stop_at) {
            while (true)
            {
                std::cout << "ding...\n";
                ++counter;
                auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                      floor<std::chrono::seconds>(std::chrono::system_clock::now()));
                if (now.get_sys_time() >= stop_at.get_sys_time())
                {
                    PF_CollectDataApp::SetSignal();
                    break;
                }
                std::this_thread::sleep_for(1s);
            }
        };

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            // add an external timer here.
            auto timer_task = std::async(std::launch::async, timer, then);

            myApp.Run();
            myApp.Shutdown();

            timer_task.get();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_TRUE(fs::exists("/tmp/test_charts_Eodhd/SPY_0.05X1_linear.json"));
}

class StreamTiingoData : public Test
{
};

TEST_F(StreamTiingoData, VerifyConnectAndDisconnect) // NOLINT
{
    if (fs::exists("/tmp/test_charts_T"))
    {
        fs::remove_all("/tmp/test_charts_T");
    }

    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--symbol", "AAPL",
        "--new-data-source", "streaming",
        "--quote-host", "api.tiingo.com",
        "--quote-data-source", "Tiingo",
        "--quote-api-key", "Tiingo_key.dat",

        "--streaming-host", "api.tiingo.com",
        "--streaming-data-source", "Tiingo",
        "--streaming-api-key", "Tiingo_key.dat",

        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts_T",
        "--boxsize", "0.1",
        "--boxsize", "0.05",
        "--reversal", "1",
        "--use-ATR",
        "-l", "debug",
        "--log-path", "/tmp/PF_Collect/test23.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                              floor<std::chrono::seconds>(std::chrono::system_clock::now()));
        auto then = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                               floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 15s);

        int counter = 0;
        auto timer = [&counter](const auto &stop_at) {
            while (true)
            {
                std::cout << "ding...\n";
                ++counter;
                auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                      floor<std::chrono::seconds>(std::chrono::system_clock::now()));
                if (now.get_sys_time() >= stop_at.get_sys_time())
                {
                    PF_CollectDataApp::SetSignal();
                    break;
                }
                std::this_thread::sleep_for(1s);
            }
        };

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            // add an external timer here.
            auto timer_task = std::async(std::launch::async, timer, then);

            myApp.Run();
            myApp.Shutdown();

            timer_task.get();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    ASSERT_TRUE(fs::exists("/tmp/test_charts_T/SPY_0.05X1_linear.json"));
}

TEST_F(StreamTiingoData, DISABLED_VerifySignalHandling) // NOLINT
{
    if (fs::exists("/tmp/test_charts"))
    {
        fs::remove_all("/tmp/test_charts");
    }
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol-list", "SPY,aapl",
        "--new-data-source", "streaming",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "0.05",
        "--reversal", "1",
        "-l", "debug",
        "--log-path", "/tmp/PF_Collect/test24.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();
        if (startup_OK)
        {
            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.05X1_linear.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.05X1_linear.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.05X1_linear.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.05X1_linear.svg"));
}

TEST_F(StreamTiingoData, TryLogarithmicCharts) // NOLINT
{
    if (fs::exists("/tmp/test_charts_log"))
    {
        fs::remove_all("/tmp/test_charts_log");
    }
    //	NOTE: the program name 'the_program' in the command line below is ignored in the
    //	the test program.

    // clang-format off
	std::vector<std::string> tokens{"the_program",
        "--symbol", "GOOG",
        "--symbol", "AAPL",
        "--new-data-source", "streaming",
        "--quote-host", "api.tiingo.com",
        "--quote-data-source", "Tiingo",
        "--quote-api-key", "Tiingo_key.dat",

        "--streaming-host", "api.tiingo.com",
        "--streaming-data-source", "Tiingo",
        "--streaming-api-key", "Tiingo_key.dat",
        "--mode", "load",
        "--interval", "live",
        "--scale", "percent",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts_log",
        "--use-ATR",
        "--boxsize", "0.01",
        "--reversal", "1",
        "-l", "debug",
        "--log-path", "/tmp/PF_Collect/test25.log"
	};
    // clang-format on

    try
    {
        PF_CollectDataApp myApp(tokens);

        const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(std::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                              floor<std::chrono::seconds>(std::chrono::system_clock::now()));
        auto then = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                               floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 15s);

        int counter = 0;
        auto timer = [&counter](const auto &stop_at) {
            while (true)
            {
                std::cout << "ding...\n";
                ++counter;
                auto now = std::chrono::zoned_seconds(std::chrono::current_zone(),
                                                      floor<std::chrono::seconds>(std::chrono::system_clock::now()));
                if (now.get_sys_time() >= stop_at.get_sys_time())
                {
                    PF_CollectDataApp::SetSignal();
                    break;
                }
                std::this_thread::sleep_for(1s);
            }
        };

        if (startup_OK)
        {
            // add an external timer here.
            auto timer_task = std::async(std::launch::async, timer, then);

            myApp.Run();
            myApp.Shutdown();
        }
        else
        {
            std::cout << "Problems starting program.  No processing done.\n";
        }
    }

    // catch any problems trying to setup application

    catch (const std::exception &theProblem)
    {
        spdlog::error(std::format("Something fundamental went wrong: {}", theProblem.what()));
    }
    catch (...)
    { // handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
    }
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/GOOG_0.01%X1_percent.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/GOOG_0.01%X1_percent.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X1_percent.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X1_percent.svg"));
}

void InitLogging()
{
    //    nothing to do for now.
    //    logging::core::get()->set_filter
    //    (
    //        logging::trivial::severity >= logging::trivial::trace
    //    );
} /* -----  end of function InitLogging  ----- */

int main(int argc, char **argv)
{
    // simpler logging setup than unit test because here
    // the app class will set up required logging.

    auto my_default_logger = spdlog::stdout_color_mt("testing_logger");
    spdlog::set_default_logger(my_default_logger);
    spdlog::set_level(spdlog::level::info);

    if (fs::exists("/tmp/PF_Collect"))
    {
        fs::remove_all("/tmp/PF_Collect");
    }

    setenv("PF_COLLECT_DATA_CONFIG_DIR", "/home/dpriedel/.config/PF_CollectData", true);

    // InitLogging();

    InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
