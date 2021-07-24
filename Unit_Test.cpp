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
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <system_error>

#include <gmock/gmock.h>


namespace fs = std::filesystem;

using namespace testing;


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


class IdentifyXMLFilesToUse : public Test
{

};

TEST_F(IdentifyXMLFilesToUse, FileNameHasForm)
{
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  InitLogging
 *  Description:  
 * =====================================================================================
 */
void InitLogging ()
{
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
//    auto file_content_10K = LoadDataFileForUse(XLS_SHEET_1);
//    EM::FileContent file_content{file_content_10K};
//
//    const auto document_sections_10K{LocateDocumentSections(file_content)};
//
//    auto xls_content = LocateXLSDocument(document_sections_10K, XLS_SHEET_1);
//
//    auto xls_data = ExtractXLSData(xls_content);
//
//   XLS_File xls_file{std::move(xls_data)};
//
//   int number_of_sheets = rng::distance(xls_file);
////   int number_of_sheets2 = rng::distance(xls_file);
//
//   auto bal_sheets = rng::find_if(xls_file, [] (const auto& x) { return x.GetSheetName() == "balance sheets"; } );
////   auto bal_sheets2 = rng::find_if(xls_file, [] (const auto& x) { return x.GetSheetName() == "balance sheets"; } );
//
////   int rows = rng::distance(*bal_sheets);
//   for (const auto& row : *bal_sheets)
//   {
//       std::cout << row << '\n';
//   }
//
//   auto stmt_of_ops = rng::find_if(xls_file, [] (const auto& x) { return x.GetSheetName() == "statements of operations"; } );
//
//   int rows2 = rng::distance(*stmt_of_ops);
//
//    return 0;
//   auto cash_flows = rng::find_if(xls_file, [] (const auto& x) { return x.GetSheetName() == "statements of cash flows"; } );
//
//   int rows3 = rng::distance(*cash_flows);
//
}
