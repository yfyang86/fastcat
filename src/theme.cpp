#include "theme.h"
#include "syntax_highlight.h"

namespace fastcat {

Theme get_vim_theme() {
    return Theme{
        .name = "vim-dark",
        .line_number_fg = Color::BRIGHT_BLACK,
        .line_number_bg = Color::BLACK,
        .text_fg = Color::WHITE,
        .text_bg = Color::BLACK,
        .keyword_fg = Color::BLUE,
        .string_fg = Color::YELLOW,
        .number_fg = Color::MAGENTA,
        .comment_fg = Color::DIM,
        .function_fg = Color::GREEN,
        .type_fg = Color::CYAN,
        .bold_keywords = true,
        .bold_strings = false,
        .bold_numbers = false,
        .bold_functions = false,
    };
}

void apply_theme_to_syntax(const Theme& theme) {
    // This would modify the syntax definitions to use theme colors
    // For now, the theme is applied during output formatting
}

}  // namespace fastcat
