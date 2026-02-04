#include "args.h"
#include "file_reader.h"
#include "syntax_highlight.h"
#include "csv_formatter.h"
#include "theme.h"
#include "pager.h"

#include <iostream>
#include <string>
#include <optional>
#include <stdexcept>
#include <unistd.h>

namespace fastcat {

// Output styled line with optional syntax highlighting
void output_styled_line(
    const std::string& line,
    std::size_t line_num,
    const std::optional<SyntaxDefinition>& syntax,
    const std::optional<Theme>& theme,
    bool line_numbers,
    bool use_pager,
    Pager* pager
) {
    std::string output;

    if (line_numbers) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%6zu  ", line_num);
        output += buf;
    }

    if (!syntax || syntax->name == "csv") {
        // No syntax highlighting, just output the line
        output += line;
        if (use_pager && pager) {
            pager->output_line(output);
        } else {
            std::cout << output << "\n";
        }
        return;
    }

    auto tokens = highlight_line(line, *syntax, false);

    for (const auto& token : tokens) {
        if (!token.color.empty()) {
            output += token.color;
        }
        if (token.bold) {
            output += Color::BOLD;
        }
        output += token.text;
        output += Color::RESET;
    }

    if (use_pager && pager) {
        pager->output_line(output);
    } else {
        std::cout << output << "\n";
    }
}

void process_file(
    const std::string& path,
    const Arguments& args,
    bool is_tty
) {
    auto reader = create_file_reader(path);
    auto file_info = reader->info();

    // Determine if we should use pager
    bool use_pager = args.pager || (is_tty && file_info.size_category == FileSize::Large);

    // Get syntax definition
    std::optional<SyntaxDefinition> syntax;
    if (args.syntax) {
        // Use specified syntax - create syntax definition from name
        syntax = SyntaxDefinition{};
        syntax->name = *args.syntax;
        // Map short names to full names
        if (syntax->name == "md") syntax->name = "markdown";
        if (syntax->name == "py") syntax->name = "python";
        if (syntax->name == "cpp" || syntax->name == "c") syntax->name = "cpp";
    } else {
        syntax = detect_syntax(path);
    }

    // Apply theme if requested
    std::optional<Theme> theme;
    if (args.theme) {
        theme = get_vim_theme();
    }

    std::unique_ptr<Pager> pager;
    if (use_pager) {
        pager = std::make_unique<Pager>(
            [](const std::string& text) { std::cout << text; },
            0,  // auto-detect page size
            args.line_numbers  // line numbers
        );
    }

    try {
        if (args.rainbow_csv) {
            // Rainbow CSV mode
            auto table = parse_csv(*reader);
            if (table) {
                auto lines = format_rainbow_csv_table(*table);
                for (const auto& line : lines) {
                    if (use_pager && pager) {
                        pager->output_line(line);
                    } else {
                        std::cout << line << "\n";
                    }
                }
            } else {
                while (auto result = reader->read_line()) {
                    if (result->is_eof) break;
                    std::cout << result->line << "\n";
                }
            }
        } else if (args.align_csv || (syntax && syntax->name == "csv")) {
            // CSV mode with table formatting
            auto table = parse_csv(*reader);
            if (table) {
                auto lines = format_csv_table(*table);
                for (const auto& line : lines) {
                    if (use_pager && pager) {
                        pager->output_line(line);
                    } else {
                        std::cout << line << "\n";
                    }
                }
            } else {
                // Fallback to regular output
                while (auto result = reader->read_line()) {
                    if (result->is_eof) break;
                    if (use_pager && pager) {
                        if (args.line_numbers) {
                            pager->output_line_number(result->line, result->line_number);
                        } else {
                            pager->output_line(result->line);
                        }
                    } else {
                        std::cout << result->line << "\n";
                    }
                }
            }
        } else if (args.align_md_table || (syntax && syntax->name == "markdown")) {
            // Markdown table alignment mode
            std::vector<std::string> all_lines;
            while (auto result = reader->read_line()) {
                if (result->is_eof) break;
                all_lines.push_back(result->line);
            }

            // Find and format markdown tables
            std::size_t i = 0;
            while (i < all_lines.size()) {
                if (looks_like_md_table(all_lines[i]) && !is_md_table_separator(all_lines[i])) {
                    // Collect consecutive table lines (skipping separators)
                    std::vector<std::string> table_lines;
                    while (i < all_lines.size() && looks_like_md_table(all_lines[i])) {
                        if (!is_md_table_separator(all_lines[i])) {
                            table_lines.push_back(all_lines[i]);
                        }
                        ++i;
                    }

                    // Format and output the table
                    auto formatted = format_md_table(table_lines);
                    for (const auto& line : formatted) {
                        std::cout << line << "\n";
                    }
                } else {
                    // Non-table line
                    std::cout << all_lines[i] << "\n";
                    ++i;
                }
            }
        } else {
            // Regular file output with optional syntax highlighting
            while (auto result = reader->read_line()) {
                if (result->is_eof) break;
                output_styled_line(result->line, result->line_number, syntax, theme, args.line_numbers, use_pager, pager.get());
            }
        }

        if (pager) {
            pager->finalize();
        }
    } catch (const std::runtime_error& e) {
        if (std::string(e.what()) != "pager_stopped") {
            throw;
        }
        // User quit the pager
    }
}

// Process stdin input
void process_stdin(const Arguments& args) {
    // Get syntax definition (use specified or default to cpp for stdin)
    std::optional<SyntaxDefinition> syntax;
    if (args.syntax) {
        syntax = SyntaxDefinition{};
        syntax->name = *args.syntax;
    }

    // Apply theme if requested
    std::optional<Theme> theme;
    if (args.theme) {
        theme = get_vim_theme();
    }

    // Collect all lines first for CSV processing
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(std::cin, line)) {
        lines.push_back(line);
    }

    if (lines.empty()) return;

    // Check if it looks like CSV
    bool looks_like_csv_data = looks_like_csv(lines[0]);

    if (args.rainbow_csv || (args.align_csv && looks_like_csv_data) || (syntax && syntax->name == "csv")) {
        // Parse as CSV
        struct StdinReader : public IFileReader {
            std::vector<std::string> lines;
            std::size_t idx = 0;
            std::size_t line_num = 0;

            StdinReader(std::vector<std::string>&& l) : lines(std::move(l)) {}

            std::optional<ReadResult> read_line() override {
                if (idx >= lines.size()) {
                    return ReadResult{"", line_num, true};
                }
                return ReadResult{lines[idx++], ++line_num, false};
            }

            bool seek(std::size_t) override { return false; }
            FileInfo info() const override { return {"/dev/stdin", 0, FileSize::Small}; }
            bool is_large() const override { return false; }
            void rewind() override { idx = 0; }
        };

        StdinReader reader(std::move(lines));
        auto table = parse_csv(reader);
        if (table) {
            auto formatted = args.rainbow_csv ? format_rainbow_csv_table(*table) : format_csv_table(*table);
            for (const auto& l : formatted) {
                std::cout << l << "\n";
            }
            return;
        }
    }

    // Check for markdown table
    bool looks_like_md = args.align_md_table || (syntax && syntax->name == "markdown");
    if (looks_like_md) {
        // Find and format markdown tables
        std::size_t i = 0;
        while (i < lines.size()) {
            if (looks_like_md_table(lines[i]) && !is_md_table_separator(lines[i])) {
                // Collect consecutive table lines (skipping separators)
                std::vector<std::string> table_lines;
                while (i < lines.size() && looks_like_md_table(lines[i])) {
                    if (!is_md_table_separator(lines[i])) {
                        table_lines.push_back(lines[i]);
                    }
                    ++i;
                }

                // Format and output the table
                auto formatted = format_md_table(table_lines);
                for (const auto& line : formatted) {
                    std::cout << line << "\n";
                }
            } else {
                // Non-table line
                std::cout << lines[i] << "\n";
                ++i;
            }
        }
        return;
    }

    // Regular line-by-line output
    std::size_t line_num = 0;
    for (const auto& l : lines) {
        ++line_num;
        output_styled_line(l, line_num, syntax, theme, args.line_numbers, false, nullptr);
    }
}

}  // namespace fastcat

int main(int argc, char* argv[]) {
    using namespace fastcat;

    // Parse command line arguments
    auto args = parse_args(argc, argv);
    if (!args) {
        return 0;  // --help was shown
    }

    // Check if output is a TTY
    bool is_tty = isatty(STDOUT_FILENO);

    // If -e flag is set, read from stdin
    if (args->echo) {
        fastcat::process_stdin(*args);
        return 0;
    }

    // Process each file
    for (const auto& path : args->files) {
        try {
            process_file(path, *args, is_tty);
        } catch (const std::exception& e) {
            std::cerr << "Error processing " << path << ": " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}
