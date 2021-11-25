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

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include <gmock/gmock.h>


namespace fs = std::filesystem;

const fs::path APPL_EOD_CSV{"./test_files/AAPL_close.dat"};

using namespace testing;

std::shared_ptr<spdlog::logger> DEFAULT_LOGGER;

class SingleFileEndToEnd : public Test
{
};


TEST_F(SingleFileEndToEnd, VerifyCanLoadCSVDataAndSaveToChartFile)
{
	//	NOTE: the program name 'the_program' in the command line below is ignored in the
	//	the test program.

	std::vector<std::string> tokens{"the_program",
        "--symbol", "SPY",
        "--source", "file",
        "--input_dir", "./test_files",
        "--source_format", "csv",
        "--mode", "load",
        "--interval", "eod",
        "--scale", "arithmetic",
        "--price_fld_name", "Close",
        "--destination", "file",
        "--output_dir", "/tmp/test_charts",
        "--boxsize", "10",
        "--reversal", "3"
	};

	try
	{
        PF_CollectDataApp myApp(tokens);

		const auto *test_info = UnitTest::GetInstance()->current_test_info();
        spdlog::info(fmt::format("\n\nTest: {}  test case: {} \n\n", test_info->name(), test_info->test_case_name()));

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
//	ASSERT_EQ(CountRows(), 55);
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

	InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
