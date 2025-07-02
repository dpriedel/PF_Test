#include "utilities.h" // For MakeHolidayList and related types
#include <chrono>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    // Validate command-line arguments
    if (argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <start_year> <number_of_years> <output_file_path>\n";
        return 1;
    }

    int start_year;
    int num_years;
    std::filesystem::path output_file_path;

    // Parse and validate start_year and number_of_years
    try
    {
        start_year = std::stoi(argv[1]);
        num_years = std::stoi(argv[2]);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << std::format("Error: Start year and number of years must be valid integers. Details: {}\n",
                                 e.what());
        return 1;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << std::format("Error: Start year or number of years out of integer range. Details: {}\n", e.what());
        return 1;
    }

    if (num_years < 1)
    {
        std::cerr << "Error: Number of years must be at least 1.\n";
        return 1;
    }

    output_file_path = argv[3];

    // Open the output file in append mode. Create it if it doesn't exist.
    std::ofstream output_file;
    output_file.open(output_file_path, std::ios::out | std::ios::app);

    if (!output_file.is_open())
    {
        std::cerr << std::format("Error: Could not open file {} for writing.\n", output_file_path);
        return 1;
    }

    std::cout << std::format("Generating holidays from year {} for {} years, writing to {}.\n", start_year, num_years,
                             output_file_path);

    // Iterate through the specified range of years
    for (int year_offset = 0; year_offset < num_years; ++year_offset)
    {
        int current_year_val = start_year + year_offset;
        std::chrono::year current_year(current_year_val);

        // Generate the list of US market holidays for the current year
        US_MarketHolidays holidays = MakeHolidayList(current_year);

        // Write holidays to the file in tab-delimited format with no extra decorations
        for (const auto &holiday : holidays)
        {
            // Format: Holiday Name <TAB> YYYY-MM-DD
            output_file << std::format("{}\t{:%Y-%m-%d}\n", holiday.first, holiday.second);
        }
    }

    output_file.close();
    std::cout << "Successfully wrote holiday list to " << output_file_path << "\n";

    return 0;
}
