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

#include <spdlog/spdlog.h>

#include <gmock/gmock.h>


namespace fs = std::filesystem;

const fs::path FILE_WITH_XML_10Q{"/vol_DA/SEC/Archives/edgar/data/1460602/0001062993-13-005017.txt"};

using namespace testing;

std::shared_ptr<spdlog::logger> DEFAULT_LOGGER;

class SingleFileEndToEnd : public Test
{
//	public:
//
//        void SetUp() override
//        {
//            spdlog::set_default_logger(DEFAULT_LOGGER);
//
//		    pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
//		    pqxx::work trxn{c};
//
//		    // make sure the DB is empty before we start
//
//		    trxn.exec("DELETE FROM unified_extracts.sec_filing_id WHERE data_source != 'HTML'");
//		    trxn.commit();
//        }
//
////		int CountRows()
////		{
////		    pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
////		    pqxx::work trxn{c};
////
////		    // make sure the DB is empty before we start
////
////		    auto row = trxn.exec1("SELECT count(*) FROM unified_extracts.sec_xbrl_data");
////		    trxn.commit();
////			return row[0].as<int>();
////		}
//		int CountRows()
//		{
//		    pqxx::connection c{"dbname=sec_extracts user=extractor_pg"};
//		    pqxx::work trxn{c};
//
//		    // make sure the DB is empty before we start
//
//		    auto row1 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join unified_extracts.sec_bal_sheet_data as t2 on t1.filing_id =  t2.filing_id where t1.data_source = 'XLS';");
//		    auto row2 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join unified_extracts.sec_stmt_of_ops_data as t2 on t1.filing_id =  t2.filing_id where t1.data_source = 'XLS';");
//		    auto row3 = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join unified_extracts.sec_cash_flows_data as t2 on t1.filing_id =  t2.filing_id where t1.data_source = 'XLS';");
//		    trxn.commit();
//			int total = row1 + row2 + row3;
//            if ( total == 0)
//            {
//                // maybe we have plain XBRL
//
//                pqxx::work trxn{c};
//                total = trxn.query_value<int>("select count(*) from unified_extracts.sec_filing_id as t1 inner join unified_extracts.sec_xbrl_data as t2 on t1.filing_id =  t2.filing_id where t1.data_source = 'XBRL';");
//                trxn.commit();
//            }
//            return total;
//		}
};


TEST_F(SingleFileEndToEnd, VerifyCanLoadDataToDBForFileWithXML10Q)
{
//	//	NOTE: the program name 'the_program' in the command line below is ignored in the
//	//	the test program.
//
//	std::vector<std::string> tokens{"the_program",
//        "--mode", "XBRL",
//        "--log-level", "debug",
//		"--form", "10-Q",
//		"-f", FILE_WITH_XML_10Q.string()
//	};
//
//	try
//	{
//        ExtractorApp myApp(tokens);
//
//		const auto *test_info = UnitTest::GetInstance()->current_test_info();
//        spdlog::info(catenate("\n\nTest: ", test_info->name(), " test case: ",
//                test_info->test_case_name(), "\n\n"));
//
//        bool startup_OK = myApp.Startup();
//        if (startup_OK)
//        {
//            myApp.Run();
//            myApp.Shutdown();
//        }
//        else
//        {
//            std::cout << "Problems starting program.  No processing done.\n";
//        }
//	}
//
//    // catch any problems trying to setup application
//
//	catch (const std::exception& theProblem)
//	{
//        spdlog::error(catenate("Something fundamental went wrong: ", theProblem.what()));
//	}
//	catch (...)
//	{		// handle exception: unspecified
//        spdlog::error("Something totally unexpected happened.");
//	}
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
