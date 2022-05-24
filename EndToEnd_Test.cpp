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

TEST_F(ProgramOptions, TestMixAndMatchOptions)
{
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
        "--chart-data-dir", "/tmp/test_charts",
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
   EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear.json"));
   ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X1_linear.json"));
}

class SingleFileEndToEnd : public Test
{
};


TEST_F(SingleFileEndToEnd, VerifyCanLoadCSVDataAndSaveToChartFile)
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear.json");
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
        "--chart-data-dir", "/tmp/test_charts",
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
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear.json"));
}

TEST_F(SingleFileEndToEnd, VerifyCanConstructChartFileFromPieces)
{
    if (fs::exists("/tmp/test_charts/SPY_10X3_linear.json"))
    {
        fs::remove("/tmp/test_charts/SPY_10X3_linear.json");
    }
    if (fs::exists("/tmp/test_charts2/SPY_10X3_linear.json"))
    {
        fs::remove("/tmp/test_charts2/SPY_10X3_linear.json");
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
        "--chart-data-dir", "/tmp/test_charts",
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
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_10X3_linear.json"));

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
        "--chart-data-dir", "/tmp/test_charts2",
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
    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear.json"));

    // now continue constructing the data file from 2 input files which, together, contain the same 
    // data as the 1 file used above. 

    PF_Chart franken_chart;

	std::vector<std::string> tokens3{"the_program",
        "--symbol", "SPY",
        "--source", "file",
        "--new-data-dir", "./test_files3",
        "--source-format", "csv",
        "--mode", "update",
        "--interval", "eod",
        "--scale", "linear",
        "--price-fld-name", "Close",
        "--destination", "file",
        "--chart-data-dir", "/tmp/test_charts2",
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

    EXPECT_TRUE(fs::exists("/tmp/test_charts2/SPY_10X3_linear.json"));
    ASSERT_TRUE(whole_chart == franken_chart);
}

class StreamData : public Test
{
};

TEST_F(StreamData, VerifyConnectAndDisconnect)
{
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
        "--chart-data-dir", "/tmp/test_charts",
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
    ASSERT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X3_linear.json"));
}

TEST_F(StreamData, VerifySignalHandling)
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
        "--chart-data-dir", "/tmp/test_charts",
        "--boxsize", "0.005",
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
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X3_linear.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/SPY_0.005X3_linear.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.005X3_linear.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts/AAPL_0.005X3_linear.svg"));
}

TEST_F(StreamData, TryLogarithmicCharts)
{
    if (fs::exists("/tmp/test_charts_log"))
    {
        fs::remove_all("/tmp/test_charts_log");
    }
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--symbol", "AAPL",
        "--source", "streaming",
        "--mode", "load",
        "--interval", "live",
        "--scale", "percent",
        "--price-fld-name", "close",
        "--destination", "file",
        "--chart-data-dir", "/tmp/test_charts_log",
        "--boxsize", "0.01",
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
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/SPY_0.01%X3_percent.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/SPY_0.01%X3_percent.svg"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X3_percent.json"));
    EXPECT_TRUE(fs::exists("/tmp/test_charts_log/AAPL_0.01%X3_percent.svg"));
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
