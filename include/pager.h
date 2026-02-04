#ifndef FASTCAT_PAGER_H
#define FASTCAT_PAGER_H

#include <string>
#include <functional>

namespace fastcat {

// Pager mode for large files
enum class PagerMode {
    Never,      // Never use pager
    Always,     // Always use pager
    Auto,       // Auto-detect based on file size/terminal
};

// Output callback for pager
using OutputCallback = std::function<void(const std::string&)>;

// Get terminal size
struct TerminalSize {
    std::size_t rows;
    std::size_t cols;
};

TerminalSize get_terminal_size();

// Detect if we should use pager
bool should_use_pager(PagerMode mode, std::uintmax_t file_size, bool is_tty);

// Simple pager that streams output with pagination
class Pager {
public:
    Pager(
        OutputCallback output,
        std::size_t page_lines = 0,
        bool line_numbers = false
    );

    void output(const std::string& text);
    void output_line(const std::string& line);
    void output_line_number(const std::string& line, std::size_t line_num);
    void flush();
    void finalize();

    std::size_t lines_output() const { return lines_output_; }

private:
    OutputCallback output_;
    std::size_t page_lines_;
    bool line_numbers_;
    std::size_t lines_output_;
    std::size_t lines_since_pause_;

    void maybe_pause();
    void wait_for_input();
};

}  // namespace fastcat

#endif  // FASTCAT_PAGER_H
