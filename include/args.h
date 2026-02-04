#ifndef FASTCAT_ARGS_H
#define FASTCAT_ARGS_H

#include <string>
#include <optional>
#include <vector>

namespace fastcat {

struct Arguments {
    std::vector<std::string> files;
    std::optional<std::string> syntax;
    bool theme = false;
    bool align_csv = false;
    bool align_md_table = false;  // Align markdown tables
    bool rainbow_csv = false;  // Rainbow CSV coloring
    bool pager = false;  // Use pager for large files (less-like)
    bool line_numbers = false;  // Enable line numbers
    bool echo = false;  // Read from stdin (pipeline mode)
    size_t pager_lines = 0;  // Number of lines per page (0 = auto-detect)
};

std::optional<Arguments> parse_args(int argc, char* argv[]);

void print_usage(const char* program_name);

}  // namespace fastcat

#endif  // FASTCAT_ARGS_H
