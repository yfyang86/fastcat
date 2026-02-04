#ifndef FASTCAT_THEME_H
#define FASTCAT_THEME_H

#include <string>

namespace fastcat {

// Theme definition
struct Theme {
    std::string name;
    std::string line_number_fg;
    std::string line_number_bg;
    std::string text_fg;
    std::string text_bg;
    std::string keyword_fg;
    std::string string_fg;
    std::string number_fg;
    std::string comment_fg;
    std::string function_fg;
    std::string type_fg;
    bool bold_keywords;
    bool bold_strings;
    bool bold_numbers;
    bool bold_functions;
};

// Get vim-like theme
Theme get_vim_theme();

// Apply theme to syntax highlighting
void apply_theme_to_syntax(const Theme& theme);

}  // namespace fastcat

#endif  // FASTCAT_THEME_H
