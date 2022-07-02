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

#include "PF_CollectDataApp.h"

#include <filesystem>
#include <future>

#include <fmt/format.h>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <pqxx/pqxx>
#include <pqxx/transaction.hxx>

#include <gmock/gmock.h>

#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;
using namespace py::literals;

using namespace std::literals::chrono_literals;
using namespace date::literals;
using namespace std::string_literals;

namespace fs = std::filesystem;

const fs::path APPL_EOD_CSV{"./test_files/AAPL_close.dat"};

using namespace testing;

std::shared_ptr<spdlog::logger> DEFAULT_LOGGER;

class ProgramOptions : public Test
{
};

TEST_F(ProgramOptions, TestMixAndMatchOptions)    //NOLINT
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_integers.json");
    }
    if (fs::exists("/tmp/test_charts/SPY_10X1_linear_integers.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X1_linear_integers.json");
    }

	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "-s", "SpY",
        "-s", "aapL",
        "--symbol", "IWr",
        "--source", "file",
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
        "--reversal", "1"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
   EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"));
   ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X1_linear_integers.json"));
}

class SingleFileEndToEnd : public Test
{
};


TEST_F(SingleFileEndToEnd, VerifyCanLoadCSVDataAndSaveToChartFile)    //NOLINT
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_integers.json");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--source", "file",
        "--new-data-dir", "./test_files3",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--reversal", "3"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"));
}

TEST_F(SingleFileEndToEnd, VerifyCanConstructChartFileFromPieces)    //NOLINT
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear_integers.json");
    }
    if (fs::exists("/tmp/test_charts2/SPY_10X3_linear_integers.json"))
    {
        fs::remove("/tmp/test_charts2/SPY_10X3_linear_integers.json");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

    // the first test run constructs the data file all at once 

    PF_Chart whole_chart;

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--source", "file",
        "--new-data-dir", "./test_files",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--reversal", "3"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear_integers.json"));

    // now construct the data file from 2 input files which, together, contain the same 
    // data as the 1 file used above. 

    PF_Chart half_chart;

	std::vector<std::string> tokens2{"the_program",
        "--symbol", "SPY",
        "--source", "file",
        "--new-data-dir", "./test_files2",
        "--source-format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts2",
        "--boxsize", "10",
        "--reversal", "3"
	};

	try
	{
        PF_CollectDataApp myApp(tokens2);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear_integers.json"));

    // now continue constructing the data file from 2 input files which, together, contain the same 
    // data as the 1 file used above. 

    PF_Chart franken_chart;

	std::vector<std::string> tokens3{"the_program",
        "--symbol", "SPY",
        "--source", "file",
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
        "--reversal", "3"
	};

	try
	{
        PF_CollectDataApp myApp(tokens3);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}

    std::cout << "\n\n whole chart:\n" << whole_chart;
    std::cout << "\n\n half chart:\n" << half_chart;
    std::cout << "\n\n franken chart:\n" << franken_chart << '\n';;

    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear_integers.json"));
    ASSERT_TRUE(whole_chart == franken_chart);
}

class LoadAndUpdate : public Test
{
};

TEST_F(LoadAndUpdate, VerifyUpdateWorksWhenNoPreviousChartData)    //NOLINT
{
    if (fs::exists("/tmp/test_charts_updates"))
    {
        fs::remove_all("/tmp/test_charts_updates");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "-s", "ADIV", "-s", "ADRU", "-s", "AIA", "-s", "AWF", "-s", "CBRL",
        "--source", "file",
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
        "--reversal", "3", "--reversal", "1"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}

    EXPECT_TRUE(fs::exists("/tmp/test_charts_updates/ADIV_0.1X3_linear_fractions.json"));
    ASSERT_TRUE(fs::exists("/tmp/test_charts_updates/CBRL_0.1X3_linear_fractions.svg"));
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

	    auto row = trxn.exec1("SELECT count(*) FROM test_point_and_figure.pf_charts");
	    trxn.commit();
		return row[0].as<int>();
	}

};

TEST_F(Database, LoadDataFromDB)    //NOLINT
{
    if (fs::exists("/tmp/test_charts"))
    {
        fs::remove_all("/tmp/test_charts");
    }

	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",      // want to use SP500 indicator but need to do more setup first
        "--symbol", "AAPL",
        "--symbol-list", "IWR,iwm,t",
        "--source", "database",
        "--mode", "load",
        "--scale", "linear",
        "--price-fld-name", "close_p",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "25",
        "--boxsize", "10",
        "--reversal", "1",
        "-r", "3",
        "--db-user", "data_updater_pg",
        "--db-name", "finance",
        "--db-data-source", "stock_data.current_data",
        "--begin-date", "2017-01-01",
        "--max-graphic-cols", "150"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X1_linear_fractions.json"));
}

class StreamData : public Test
{
};

TEST_F(StreamData, VerifyConnectAndDisconnect)    //NOLINT
{
    if (fs::exists("/tmp/test_charts"))
    {
        fs::remove_all("/tmp/test_charts");
    }

	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--symbol", "AAPL",
        "--source", "streaming",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "0.01",
        "--boxsize", "0.005",
        "--reversal", "1"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        auto now = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()));
        auto then = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 15s);

        int counter = 0;
        auto timer = [&counter] (const auto& stop_at)
            { 
                while (true)
                {
                    std::cout << "ding...\n";
                    ++counter;
                    auto now = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()));
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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X1_linear_fractions.json"));
}

TEST_F(StreamData, VerifySignalHandling)    //NOLINT
{
    if (fs::exists("/tmp/test_charts"))
    {
        fs::remove_all("/tmp/test_charts");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol-list", "SPY,aapl",
        "--source", "streaming",
        "--mode", "load",
        "--interval", "live",
        "--scale", "linear",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts",
        "--boxsize", "0.005",
        "--reversal", "1"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X1_linear_fractions.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X1_linear_fractions.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.005X1_linear_fractions.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.005X1_linear_fractions.svg"));
}

TEST_F(StreamData, TryLogarithmicCharts)    //NOLINT
{
    if (fs::exists("/tmp/test_charts_log"))
    {
        fs::remove_all("/tmp/test_charts_log");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "GOOG",
        "--symbol", "AAPL",
        "--source", "streaming",
        "--mode", "load",
        "--interval", "live",
        "--scale", "percent",
        "--price-fld-name", "close",
        "--destination", "file",
        "--output-chart-dir", "/tmp/test_charts_log",
        "--boxsize", "0.01",
        "--reversal", "1"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_suite_name()));

        bool startup_OK = myApp.Startup();

        auto now = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()));
        auto then = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()) + 15s);

        int counter = 0;
        auto timer = [&counter] (const auto& stop_at)
            { 
                while (true)
                {
                    std::cout << "ding...\n";
                    ++counter;
                    auto now = date::zoned_seconds(date::current_zone(), floor<std::chrono::seconds>(std::chrono::system_clock::now()));
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

	catch (const std::exception& theProblem)
	{
        spdlog::error(fmt::format("Something fundamental went wrong: {}", theProblem.what()));
	}
	catch (...)
	{		// handle exception: unspecified
        spdlog::error("Something totally unexpected happened.");
	}
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/GOOG_0.01%X1_percent_fractions.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/GOOG_0.01%X1_percent_fractions.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X1_percent_fractions.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X1_percent_fractions.svg"));
}


void InitLogging ()
{
    DEFAULT_LOGGER = spdlog::default_logger();

    //    nothing to do for now.
//    logging::core::get()->set_filter
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
        import PF_DrawChart
        )"
    );
	InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
