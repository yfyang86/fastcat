#include "args.h"
#include <iostream>
#include <algorithm>
#include <cstring>

namespace fastcat {

std::optional<Arguments> parse_args(int argc, char* argv[]) {
    Arguments args;

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];

        if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0) {
            print_usage(argv[0]);
            return std::nullopt;
        }

        if (strcmp(arg, "--theme") == 0) {
            args.theme = true;
            continue;
        }

        if (strcmp(arg, "--align-csv") == 0 || strcmp(arg, "--csv-table") == 0) {
            args.align_csv = true;
            continue;
        }

        if (strcmp(arg, "--align-md-table") == 0 || strcmp(arg, "--md-table") == 0) {
            args.align_md_table = true;
            continue;
        }

        if (strcmp(arg, "--pager") == 0 || strcmp(arg, "-p") == 0) {
            args.pager = true;
            continue;
        }

        if (strcmp(arg, "--no-pager") == 0) {
            args.pager = false;
            continue;
        }

        if (strcmp(arg, "--linenumber") == 0 || strcmp(arg, "-n") == 0) {
            args.line_numbers = true;
            continue;
        }

        if (strcmp(arg, "--rainbowcsv") == 0 || strcmp(arg, "--rainbow") == 0) {
            args.rainbow_csv = true;
            continue;
        }

        if (strcmp(arg, "-e") == 0) {
            args.echo = true;
            continue;
        }

        if (strcmp(arg, "--syntax") == 0 || strcmp(arg, "-s") == 0) {
            if (i + 1 >= argc) {
                std::cerr << "Error: --syntax requires a value\n";
                return std::nullopt;
            }
            args.syntax = argv[++i];
            continue;
        }

        // Treat as file name
        if (arg[0] != '-') {
            args.files.push_back(arg);
            continue;
        }

        std::cerr << "Error: Unknown option: " << arg << "\n";
        return std::nullopt;
    }

    // If no files specified, read from stdin
    if (args.files.empty()) {
        args.files.push_back("-");
    }

    return args;
}

void print_usage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] [FILES...]\n\n"
              << "Options:\n"
              << "  --help, -h          Show this help message\n"
              << "  --theme             Enable vim-like theme (bold, colors)\n"
              << "  --syntax <type>     Enable syntax highlighting (c, py, md, csv, etc.)\n"
              << "  --align-csv         Align and display CSV as table (implies --syntax csv)\n"
              << "  --align-md-table    Align markdown tables\n"
              << "  --rainbowcsv        Rainbow CSV with colored columns (256-color)\n"
              << "  --pager, -p         Use pager for output (less-like mode)\n"
              << "  --no-pager          Never use pager\n"
              << "  --linenumber, -n    Show line numbers\n"
              << "  -e                  Read from stdin (pipeline mode)\n\n"
              << "Examples:\n"
              << "  " << program_name << " file.txt\n"
              << "  " << program_name << " --theme --syntax py script.py\n"
              << "  " << program_name << " --align-csv data.csv\n"
              << "  " << program_name << " --rainbowcsv data.csv\n"
              << "  " << program_name << " -n file.txt\n"
              << "  echo 'code' | " << program_name << " -e --syntax cpp\n";
}

}  // namespace fastcat
