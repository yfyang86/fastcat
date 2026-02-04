#include "csv_formatter.h"
#include "file_reader.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace fastcat {

bool looks_like_csv(const std::string& line) {
    // Simple heuristic: contains commas and not just a few
    std::size_t comma_count = std::count(line.begin(), line.end(), ',');
    std::size_t other_chars = line.length() - comma_count;
    return comma_count > 0 && other_chars > 0;
}

CsvRow parse_csv_row(const std::string& line) {
    CsvRow row;
    std::size_t col = 0;
    std::size_t i = 0;

    while (i < line.length()) {
        std::string value;
        bool in_quotes = false;

        if (line[i] == '"') {
            in_quotes = true;
            ++i;
        }

        while (i < line.length()) {
            if (in_quotes) {
                if (line[i] == '"') {
                    if (i + 1 < line.length() && line[i + 1] == '"') {
                        // Escaped quote
                        value += '"';
                        i += 2;
                    } else {
                        in_quotes = false;
                        ++i;
                    }
                } else {
                    value += line[i];
                    ++i;
                }
            } else {
                if (line[i] == ',') {
                    break;
                }
                value += line[i];
                ++i;
            }
        }

        row.push_back(CsvCell{value, 0, col++});

        if (i < line.length() && line[i] == ',') {
            ++i;
        }
    }

    return row;
}

std::optional<CsvTable> parse_csv(
    IFileReader& reader,
    std::size_t max_rows
) {
    CsvTable table;
    std::size_t row_num = 0;

    // First pass: collect all rows and compute column widths
    while (auto result = reader.read_line()) {
        if (result->is_eof) break;

        if (max_rows > 0 && row_num >= max_rows) break;

        CsvRow row = parse_csv_row(result->line);
        row_num++;

        // Update column widths
        for (std::size_t i = 0; i < row.size(); ++i) {
            std::size_t width = row[i].value.length();
            if (i >= table.col_widths.size()) {
                table.col_widths.push_back(width);
            } else {
                table.col_widths[i] = std::max(table.col_widths[i], width);
            }
        }

        table.rows.push_back(std::move(row));
    }

    if (table.rows.empty()) {
        return std::nullopt;
    }

    table.num_cols = table.col_widths.size();
    table.num_rows = table.rows.size();

    return table;
}

std::vector<std::string> format_csv_table(const CsvTable& table) {
    std::vector<std::string> lines;

    // Build separator line
    std::string separator = "+";
    for (std::size_t i = 0; i < table.col_widths.size(); ++i) {
        separator += std::string(table.col_widths[i] + 2, '-');
        separator += "+";
    }
    lines.push_back(separator);

    // Format each row
    for (const auto& row : table.rows) {
        std::string line = "|";
        for (std::size_t i = 0; i < row.size(); ++i) {
            std::ostringstream oss;
            oss << " " << std::left << std::setw(table.col_widths[i]) << row[i].value << " |";
            line += oss.str();
        }
        lines.push_back(line);
        lines.push_back(separator);
    }

    return lines;
}

// Generate 256-color for column (rainbow effect)
std::string get_rainbow_color(std::size_t col_index) {
    // Use a subset of 256 colors for pleasant rainbow effect
    // Colors 1-6 are red, orange, yellow, green, blue, purple base colors
    // We spread them across the palette for variety
    static const std::size_t num_colors = 12;
    std::size_t color_idx = col_index % num_colors;

    // 256-color ANSI escape sequence: \033[38;5;Nm
    // Using a curated set of distinct colors
    std::size_t color_codes[] = {
        196,  // Red
        202,  // Orange-Red
        208,  // Orange
        214,  // Yellow-Orange
        220,  // Yellow
        226,  // Lemon Yellow
        46,   // Green
        47,   // Medium Spring Green
        39,   // Deep Sky Blue
        45,   // Royal Blue
        165,  // Orange-Purple
        171,  // Medium Orchid
    };

    std::ostringstream oss;
    oss << "\033[38;5;" << color_codes[color_idx] << "m";
    return oss.str();
}

std::vector<std::string> format_rainbow_csv_table(const CsvTable& table) {
    std::vector<std::string> lines;
    const std::string reset = "\033[0m";

    // Build separator line (dim gray)
    std::string separator = "\033[90m+";
    for (std::size_t i = 0; i < table.col_widths.size(); ++i) {
        separator += std::string(table.col_widths[i] + 2, '-');
        separator += "+";
    }
    separator += reset;
    lines.push_back(separator);

    // Format each row with rainbow columns
    for (const auto& row : table.rows) {
        std::string line;
        for (std::size_t i = 0; i < row.size(); ++i) {
            std::string color = get_rainbow_color(i);
            std::ostringstream oss;
            oss << "|" << color << " " << std::left << std::setw(table.col_widths[i])
                << row[i].value << " " << reset;
            line += oss.str();
        }
        line += "|";
        lines.push_back(line);
        lines.push_back(separator);
    }

    return lines;
}

// Detect if a line looks like a markdown table row
bool looks_like_md_table(const std::string& line) {
    // Must start and end with | (or just have pipes)
    // And have at least one pipe in between
    std::size_t pipe_count = std::count(line.begin(), line.end(), '|');

    // Check for markdown table separator pattern: |---|---|
    // This is the second row of a markdown table
    std::string trimmed = line;
    // Trim leading and trailing spaces
    std::size_t start = 0;
    while (start < trimmed.length() && (trimmed[start] == ' ' || trimmed[start] == '\t')) ++start;
    std::size_t end = trimmed.length();
    while (end > start && (trimmed[end - 1] == ' ' || trimmed[end - 1] == '\t')) --end;
    trimmed = trimmed.substr(start, end - start);

    // Check if it's a separator row: only |, -, :, and spaces
    if (!trimmed.empty() && trimmed[0] == '|') {
        bool is_separator = true;
        for (char c : trimmed) {
            if (c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t') {
                is_separator = false;
                break;
            }
        }
        if (is_separator) return true;
    }

    // Regular table row: at least 2 pipes (3 cells minimum)
    return pipe_count >= 2;
}

// Check if a line is a markdown table separator (only | - : and spaces)
bool is_md_table_separator(const std::string& line) {
    std::string trimmed = line;
    // Trim leading and trailing spaces
    std::size_t start = 0;
    while (start < trimmed.length() && (trimmed[start] == ' ' || trimmed[start] == '\t')) ++start;
    std::size_t end = trimmed.length();
    while (end > start && (trimmed[end - 1] == ' ' || trimmed[end - 1] == '\t')) --end;
    trimmed = trimmed.substr(start, end - start);

    if (trimmed.empty() || trimmed[0] != '|') return false;

    // Check if it's only |, -, :, and spaces
    for (char c : trimmed) {
        if (c != '|' && c != '-' && c != ':' && c != ' ' && c != '\t') {
            return false;
        }
    }
    return true;
}

// Parse markdown table rows from lines
std::vector<std::string> parse_md_table_lines(const std::vector<std::string>& lines) {
    std::vector<std::string> table_lines;

    for (const auto& line : lines) {
        // Skip separator lines - we'll generate our own
        if (is_md_table_separator(line)) {
            continue;
        }
        if (looks_like_md_table(line)) {
            table_lines.push_back(line);
        } else {
            // When we hit a non-table line, stop processing
            // (markdown tables are contiguous)
            break;
        }
    }

    return table_lines;
}

// Parse a single markdown table row into cells
std::vector<std::string> parse_md_row(const std::string& line) {
    std::vector<std::string> cells;

    std::size_t start = 0;
    std::size_t len = line.length();

    // Find first |
    std::size_t first_pipe = line.find('|');
    if (first_pipe == std::string::npos) {
        cells.push_back(line);
        return cells;
    }

    // If first char is not |, skip to first |
    if (first_pipe != 0) {
        start = first_pipe;
    } else {
        start = 1;  // Skip leading |
    }

    while (start < len) {
        std::size_t pipe_pos = line.find('|', start);
        std::string cell;

        if (pipe_pos == std::string::npos) {
            cell = line.substr(start);
            // Trim trailing spaces from last cell
            while (!cell.empty() && (cell.back() == ' ' || cell.back() == '\t')) {
                cell.pop_back();
            }
            cells.push_back(cell);
            break;
        } else {
            cell = line.substr(start, pipe_pos - start);
            // Trim spaces
            while (!cell.empty() && cell.front() == ' ') cell.erase(cell.begin());
            while (!cell.empty() && cell.back() == ' ') cell.pop_back();
            cells.push_back(cell);
            start = pipe_pos + 1;
        }
    }

    return cells;
}

// Format markdown table with aligned columns
std::vector<std::string> format_md_table(const std::vector<std::string>& table_lines) {
    if (table_lines.empty()) {
        return {};
    }

    std::vector<std::vector<std::string>> rows;
    std::vector<std::size_t> col_widths;

    // Parse all rows
    for (const auto& line : table_lines) {
        auto cells = parse_md_row(line);
        if (cells.empty()) continue;

        // Update column widths
        for (std::size_t i = 0; i < cells.size(); ++i) {
            std::size_t width = cells[i].length();
            if (i >= col_widths.size()) {
                col_widths.push_back(width);
            } else {
                col_widths[i] = std::max(col_widths[i], width);
            }
        }
        rows.push_back(std::move(cells));
    }

    if (rows.empty()) {
        return {};
    }

    std::vector<std::string> formatted;

    // Helper to pad string to width
    auto pad = [](const std::string& s, std::size_t width) -> std::string {
        if (s.length() >= width) return s;
        return s + std::string(width - s.length(), ' ');
    };

    // Build separator line with proper column widths
    // Format: |<dashes>|<dashes>|... (matches visual width of each column)
    auto build_separator = [&]() -> std::string {
        std::string sep;
        for (std::size_t i = 0; i < col_widths.size(); ++i) {
            // Row visual format is "| " + content + " " = col_widths + 3
            // But we want separator to match exactly, so use col_widths + 2
            sep += "|" + std::string(col_widths[i] + 2, '-');
        }
        sep += "|";
        return sep;
    };

    // Format each row
    for (std::size_t r = 0; r < rows.size(); ++r) {
        const auto& row = rows[r];
        std::string line;
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i == 0) {
                // First cell - may not have leading |
                line += "| ";
            } else {
                line += " | ";
            }
            line += pad(row[i], col_widths[i]);
            line += " ";  // trailing space
        }
        // Pad remaining columns
        for (std::size_t i = row.size(); i < col_widths.size(); ++i) {
            line += " | ";
            line += std::string(col_widths[i], ' ');
            line += " ";
        }
        line += "|";
        formatted.push_back(line);

        // Insert separator after header row (first row)
        if (r == 0 && rows.size() > 1) {
            formatted.push_back(build_separator());
        }
    }

    return formatted;
}

}  // namespace fastcat
