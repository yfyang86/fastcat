#ifndef FASTCAT_SYNTAX_HIGHLIGHT_H
#define FASTCAT_SYNTAX_HIGHLIGHT_H

#include <string>
#include <vector>
#include <optional>

namespace fastcat {

// ANSI color codes
struct Color {
    static constexpr const char* RESET = "\033[0m";
    static constexpr const char* BOLD = "\033[1m";
    static constexpr const char* DIM = "\033[2m";
    static constexpr const char* ITALIC = "\033[3m";
    static constexpr const char* UNDERLINE = "\033[4m";

    // Foreground colors
    static constexpr const char* BLACK = "\033[30m";
    static constexpr const char* RED = "\033[31m";
    static constexpr const char* GREEN = "\033[32m";
    static constexpr const char* YELLOW = "\033[33m";
    static constexpr const char* BLUE = "\033[34m";
    static constexpr const char* MAGENTA = "\033[35m";
    static constexpr const char* CYAN = "\033[36m";
    static constexpr const char* WHITE = "\033[37m";

    // Bright foreground colors
    static constexpr const char* BRIGHT_BLACK = "\033[90m";
    static constexpr const char* BRIGHT_RED = "\033[91m";
    static constexpr const char* BRIGHT_GREEN = "\033[92m";
    static constexpr const char* BRIGHT_YELLOW = "\033[93m";
    static constexpr const char* BRIGHT_BLUE = "\033[94m";
    static constexpr const char* BRIGHT_MAGENTA = "\033[95m";
    static constexpr const char* BRIGHT_CYAN = "\033[96m";
    static constexpr const char* BRIGHT_WHITE = "\033[97m";
};

struct SyntaxToken {
    std::string text;
    std::string color;
    bool bold;
};

// Syntax highlighting rules
struct SyntaxRule {
    std::string pattern;  // Regex pattern
    std::string color;
    bool bold;
    bool multiline;
};

// Syntax definition for a language
struct SyntaxDefinition {
    std::string name;
    std::vector<std::string> extensions;
    std::vector<SyntaxRule> rules;
    std::optional<std::string> single_line_comment;
    std::optional<std::string> multi_line_comment_start;
    std::optional<std::string> multi_line_comment_end;
};

// Detect syntax from file extension
std::optional<SyntaxDefinition> detect_syntax(const std::string& filename);

// Tokenize a line with syntax highlighting
std::vector<SyntaxToken> highlight_line(
    const std::string& line,
    const SyntaxDefinition& syntax,
    bool in_multiline_comment
);

}  // namespace fastcat

#endif  // FASTCAT_SYNTAX_HIGHLIGHT_H
