#include "pager.h"
#include <iostream>
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>

namespace fastcat {

TerminalSize get_terminal_size() {
    TerminalSize size{24, 80};

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        if (w.ws_row > 0) size.rows = w.ws_row;
        if (w.ws_col > 0) size.cols = w.ws_col;
    }

    return size;
}

bool should_use_pager(PagerMode mode, std::uintmax_t file_size, bool is_tty) {
    switch (mode) {
        case PagerMode::Always:
            return true;
        case PagerMode::Never:
            return false;
        case PagerMode::Auto:
        default:
            // Use pager if output is a tty and file is large
            return is_tty && file_size > 1024 * 1024;  // > 1MB
    }
}

Pager::Pager(
    OutputCallback output,
    std::size_t page_lines,
    bool line_numbers
)
    : output_(std::move(output))
    , page_lines_(page_lines)
    , line_numbers_(line_numbers)
    , lines_output_(0)
    , lines_since_pause_(0)
{
    if (page_lines_ == 0) {
        auto size = get_terminal_size();
        page_lines_ = size.rows > 0 ? size.rows - 2 : 20;
    }
}

void Pager::output(const std::string& text) {
    output_(text);
}

void Pager::output_line(const std::string& line) {
    output_(line);
    output_("\n");
    ++lines_output_;
    ++lines_since_pause_;
    maybe_pause();
}

void Pager::output_line_number(const std::string& line, std::size_t line_num) {
    if (line_numbers_) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%6zu  ", line_num);
        output_(buf);
    }
    output_(line);
    output_("\n");
    ++lines_output_;
    ++lines_since_pause_;
    maybe_pause();
}

void Pager::flush() {
    std::cout.flush();
}

void Pager::finalize() {
    // Ensure all output is shown
    flush();
}

void Pager::maybe_pause() {
    if (lines_since_pause_ >= page_lines_) {
        wait_for_input();
        lines_since_pause_ = 0;
    }
}

void Pager::wait_for_input() {
    output_("\033[7m-- More --\033[0m");
    std::cout.flush();

    // Read single character without echo
    struct termios old_settings, new_settings;
    tcgetattr(STDIN_FILENO, &old_settings);
    new_settings = old_settings;
    new_settings.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_settings);

    char c = 0;
    ssize_t n = read(STDIN_FILENO, &c, 1);

    tcsetattr(STDIN_FILENO, TCSANOW, &old_settings);

    // Clear the "More" message
    output_("\033[1G\033[K");
    std::cout.flush();

    if (n <= 0 || c == 'q' || c == 'Q' || c == 27) {
        throw std::runtime_error("pager_stopped");
    }
}

}  // namespace fastcat
