#ifndef FASTCAT_CSV_FORMATTER_H
#define FASTCAT_CSV_FORMATTER_H

#include <string>
#include <vector>
#include <optional>
#include "file_reader.h"

namespace fastcat {

// CSV cell with metadata
struct CsvCell {
    std::string value;
    std::size_t row;
    std::size_t col;
};

// CSV row
using CsvRow = std::vector<CsvCell>;

// CSV table with metadata
struct CsvTable {
    std::vector<CsvRow> rows;
    std::vector<std::size_t> col_widths;
    std::size_t num_cols;
    std::size_t num_rows;
};

// Detect if a line looks like CSV
bool looks_like_csv(const std::string& line);

// Parse a single CSV row
CsvRow parse_csv_row(const std::string& line);

// Parse CSV file and return formatted table
// Returns std::nullopt if not valid CSV
std::optional<CsvTable> parse_csv(
    IFileReader& reader,
    std::size_t max_rows = 0  // 0 = all rows
);

// Format CSV table for display with alignment
std::vector<std::string> format_csv_table(const CsvTable& table);

// Generate 256-color for column (rainbow effect)
std::string get_rainbow_color(std::size_t col_index);

// Format CSV table with rainbow column coloring
std::vector<std::string> format_rainbow_csv_table(const CsvTable& table);

// Detect if a line looks like a markdown table row
bool looks_like_md_table(const std::string& line);

// Check if a line is a markdown table separator (only | - : and spaces)
bool is_md_table_separator(const std::string& line);

// Parse markdown table rows from lines
// Returns vector of table lines (header, separator, data rows)
std::vector<std::string> parse_md_table_lines(const std::vector<std::string>& lines);

// Format markdown table with aligned columns
std::vector<std::string> format_md_table(const std::vector<std::string>& table_lines);

}  // namespace fastcat

#endif  // FASTCAT_CSV_FORMATTER_H
