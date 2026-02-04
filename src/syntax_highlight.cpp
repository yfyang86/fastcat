#include "syntax_highlight.h"
#include <regex>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace fastcat {

// Find pattern in line and return position, or std::string::npos
std::size_t find_pattern(const std::string& line, const char* pattern) {
    return line.find(pattern);
}

// Check if line starts with a prefix
bool starts_with(const std::string& line, const char* prefix) {
    return line.compare(0, strlen(prefix), prefix) == 0;
}

// Create C++ syntax definition
SyntaxDefinition create_cpp_syntax() {
    SyntaxDefinition syntax;
    syntax.name = "cpp";
    syntax.extensions = {".cpp", ".hpp", ".cxx", ".hxx", ".cc", ".hh", ".C", ".h"};
    syntax.single_line_comment = "//";
    syntax.multi_line_comment_start = "/*";
    syntax.multi_line_comment_end = "*/";
    return syntax;
}

// Create Python syntax definition
SyntaxDefinition create_python_syntax() {
    SyntaxDefinition syntax;
    syntax.name = "python";
    syntax.extensions = {".py", ".pyw"};
    syntax.single_line_comment = "#";
    return syntax;
}

// Create Markdown syntax definition
SyntaxDefinition create_markdown_syntax() {
    SyntaxDefinition syntax;
    syntax.name = "markdown";
    syntax.extensions = {".md", ".markdown"};
    syntax.multi_line_comment_start = "```";
    syntax.multi_line_comment_end = "```";
    return syntax;
}

// Create CSV syntax definition
SyntaxDefinition create_csv_syntax() {
    SyntaxDefinition syntax;
    syntax.name = "csv";
    syntax.extensions = {".csv", ".tsv"};
    return syntax;
}

// Create JSON syntax definition
SyntaxDefinition create_json_syntax() {
    SyntaxDefinition syntax;
    syntax.name = "json";
    syntax.extensions = {".json"};
    return syntax;
}

std::optional<SyntaxDefinition> detect_syntax(const std::string& filename) {
    std::string ext;
    auto pos = filename.rfind('.');
    if (pos != std::string::npos) {
        ext = filename.substr(pos);
    }

    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Check each syntax definition
    auto cpp = create_cpp_syntax();
    auto py = create_python_syntax();
    auto md = create_markdown_syntax();
    auto csv = create_csv_syntax();
    auto json = create_json_syntax();

    for (const auto& syntax : {cpp, py, md, csv, json}) {
        for (const auto& syntax_ext : syntax.extensions) {
            if (ext == syntax_ext) {
                return syntax;
            }
        }
    }

    // Default: no syntax highlighting
    return std::nullopt;
}

// Highlight C++ code
std::vector<SyntaxToken> highlight_cpp(const std::string& line) {
    std::vector<SyntaxToken> tokens;
    std::size_t pos = 0;

    // Preprocessor directives
    if (starts_with(line, "#include") || starts_with(line, "#define") ||
        starts_with(line, "#ifdef") || starts_with(line, "#ifndef") ||
        starts_with(line, "#endif") || starts_with(line, "#else") ||
        starts_with(line, "#elif") || starts_with(line, "#pragma")) {
        // Highlight entire line as preprocessor
        tokens.push_back(SyntaxToken{line, Color::GREEN, false});
        return tokens;
    }

    // String literals
    std::size_t str_start = line.find('"');
    while (str_start != std::string::npos) {
        // Add text before string
        if (str_start > pos) {
            tokens.push_back(SyntaxToken{line.substr(pos, str_start - pos), "", false});
        }
        // Find end of string (handle escaped quotes)
        std::size_t search_from = str_start + 1;
        while (search_from < line.length()) {
            if (line[search_from] == '\\' && search_from + 1 < line.length()) {
                search_from += 2;
            } else if (line[search_from] == '"') {
                break;
            } else {
                ++search_from;
            }
        }
        std::size_t str_end = (search_from < line.length()) ? search_from + 1 : line.length();
        tokens.push_back(SyntaxToken{line.substr(str_start, str_end - str_start), Color::YELLOW, false});
        pos = str_end;
        str_start = line.find('"', search_from);
    }

    // Character literals
    std::size_t char_start = line.find('\'');
    while (char_start != std::string::npos) {
        if (char_start > pos) {
            tokens.push_back(SyntaxToken{line.substr(pos, char_start - pos), "", false});
        }
        std::size_t search_from = char_start + 1;
        while (search_from < line.length()) {
            if (line[search_from] == '\\' && search_from + 1 < line.length()) {
                search_from += 2;
            } else if (line[search_from] == '\'') {
                break;
            } else {
                ++search_from;
            }
        }
        std::size_t char_end = (search_from < line.length()) ? search_from + 1 : line.length();
        tokens.push_back(SyntaxToken{line.substr(char_start, char_end - char_start), Color::YELLOW, false});
        pos = char_end;
        char_start = line.find('\'', search_from);
    }

    // Single-line comment
    auto comment_pos = line.find("//");
    if (comment_pos != std::string::npos) {
        if (comment_pos > pos) {
            tokens.push_back(SyntaxToken{line.substr(pos, comment_pos - pos), "", false});
        }
        tokens.push_back(SyntaxToken{line.substr(comment_pos), Color::DIM, false});
        return tokens;
    }

    // Remaining text
    if (pos < line.length()) {
        std::string remaining = line.substr(pos);

        // Simple keyword detection
        const char* keywords[] = {
            "int", "long", "short", "float", "double", "char", "void", "bool",
            "auto", "const", "static", "extern", "struct", "class", "enum",
            "union", "public", "private", "protected", "virtual", "override",
            "final", "inline", "constexpr", "mutable", "sizeof", "typedef",
            "namespace", "template", "typename", "using", "delete", "noexcept",
            "static_assert", "decltype", "return", "if", "else", "for", "while",
            "do", "switch", "case", "break", "continue", "new", "this", "try",
            "catch", "throw", "nullptr", "true", "false", "NULL", "explicit"
        };

        std::string result;
        std::size_t rpos = 0;
        while (rpos < remaining.length()) {
            // Check for keyword
            bool is_keyword = false;
            for (const char* kw : keywords) {
                std::size_t klen = strlen(kw);
                if (remaining.compare(rpos, klen, kw) == 0) {
                    // Check word boundary
                    char prev = (rpos > 0) ? remaining[rpos - 1] : ' ';
                    char next = (rpos + klen < remaining.length()) ? remaining[rpos + klen] : ' ';
                    if (!std::isalnum(prev) && !std::isalnum(next) && prev != '_' && next != '_') {
                        if (rpos < remaining.length() && !result.empty()) {
                            result += Color::RESET;
                        }
                        result += Color::BLUE;
                        result += Color::BOLD;
                        result += kw;
                        result += Color::RESET;
                        rpos += klen;
                        is_keyword = true;
                        break;
                    }
                }
            }
            if (!is_keyword) {
                result += remaining[rpos];
                ++rpos;
            }
        }

        tokens.push_back(SyntaxToken{result, "", false});
    }

    if (tokens.empty()) {
        tokens.push_back(SyntaxToken{line, "", false});
    }
    return tokens;
}

// Highlight Markdown
std::vector<SyntaxToken> highlight_markdown(const std::string& line) {
    std::vector<SyntaxToken> tokens;
    std::size_t pos = 0;
    std::size_t len = line.length();

    // Skip leading whitespace for pattern matching
    std::size_t content_start = 0;
    while (content_start < len && (line[content_start] == ' ' || line[content_start] == '\t')) {
        ++content_start;
    }

    // Block quote
    if (content_start < len && line[content_start] == '>') {
        tokens.push_back(SyntaxToken{line.substr(0, content_start + 1), Color::DIM, false});
        tokens.push_back(SyntaxToken{line.substr(content_start + 1), Color::CYAN, false});
        return tokens;
    }

    // Code block start/end (```)
    std::size_t first_non_space = content_start;
    while (first_non_space + 3 <= len) {
        if (line.compare(first_non_space, 3, "```") == 0) {
            // Highlight the entire backtick line
            if (first_non_space > 0) {
                tokens.push_back(SyntaxToken{line.substr(0, first_non_space), "", false});
            }
            tokens.push_back(SyntaxToken{line.substr(first_non_space), Color::GREEN, true});
            return tokens;
        }
        if (line[first_non_space] != ' ' && line[first_non_space] != '\t') break;
        ++first_non_space;
    }

    // Indented code block (4+ spaces or 1+ tab at start)
    bool is_indented_code = false;
    if (content_start >= 4 || (line.find('\t') != std::string::npos && content_start > 0)) {
        is_indented_code = true;
    }

    // Headers (# ## ###)
    if (content_start < len && line[content_start] == '#') {
        std::size_t hash_count = 0;
        std::size_t hpos = content_start;
        while (hpos < len && line[hpos] == '#' && hash_count < 6) {
            ++hash_count;
            ++hpos;
        }
        if (hash_count > 0 && hpos < len && line[hpos] == ' ') {
            // Highlight headers in blue with bold
            std::string header_text = line.substr(content_start, hpos + 1);
            tokens.push_back(SyntaxToken{header_text, Color::BLUE, true});
            tokens.push_back(SyntaxToken{line.substr(hpos + 1), "", false});
            return tokens;
        }
    }

    // Bullet points (-, *, +)
    if (content_start < len && (line[content_start] == '-' || line[content_start] == '*' || line[content_start] == '+')) {
        if (content_start + 1 < len && line[content_start + 1] == ' ') {
            // Highlight bullet marker in green
            tokens.push_back(SyntaxToken{line.substr(0, content_start + 2), Color::GREEN, true});
            tokens.push_back(SyntaxToken{line.substr(content_start + 2), "", false});
            return tokens;
        }
    }

    // Numbered lists (1. 2. etc)
    if (content_start < len && std::isdigit(line[content_start])) {
        std::size_t num_end = content_start;
        while (num_end < len && std::isdigit(line[num_end])) ++num_end;
        if (num_end < len && line[num_end] == '.' && num_end + 1 < len && line[num_end + 1] == ' ') {
            tokens.push_back(SyntaxToken{line.substr(content_start, num_end - content_start + 2), Color::GREEN, false});
            tokens.push_back(SyntaxToken{line.substr(num_end + 2), "", false});
            return tokens;
        }
    }

    // Table row (check if line looks like a markdown table)
    bool looks_like_table = false;
    if (content_start < len && line[content_start] == '|') {
        looks_like_table = true;
    } else if (len > 0) {
        std::size_t pipe_count = std::count(line.begin() + content_start, line.end(), '|');
        if (pipe_count >= 2) looks_like_table = true;
    }

    if (looks_like_table) {
        std::size_t current = content_start;
        while (current < len) {
            if (line[current] == '|') {
                tokens.push_back(SyntaxToken{std::string(1, line[current]), Color::BRIGHT_RED, true});
                ++current;
            } else if (line[current] == '-' || line[current] == ':') {
                // Table separator
                std::size_t sep_start = current;
                while (current < len && (line[current] == '-' || line[current] == ':' || line[current] == ' ')) {
                    ++current;
                }
                tokens.push_back(SyntaxToken{line.substr(sep_start, current - sep_start), Color::DIM, false});
            } else {
                std::size_t text_start = current;
                while (current < len && line[current] != '|') ++current;
                tokens.push_back(SyntaxToken{line.substr(text_start, current - text_start), "", false});
            }
        }
        return tokens;
    }

    // Regular line with inline formatting
    std::size_t i = content_start;
    while (i < len) {
        // Check for inline code `code`
        if (line[i] == '`') {
            if (i > pos) {
                tokens.push_back(SyntaxToken{line.substr(pos, i - pos), "", false});
            }
            std::size_t code_start = i;
            ++i;
            std::size_t code_end = line.find('`', i);
            if (code_end == std::string::npos) {
                code_end = len;
            } else {
                ++code_end;
            }
            tokens.push_back(SyntaxToken{line.substr(code_start, code_end - code_start), Color::YELLOW, false});
            pos = i = code_end;
        }
        // Check for bold **text**
        else if (line[i] == '*' && i + 1 < len && line[i + 1] == '*') {
            if (i > pos) {
                tokens.push_back(SyntaxToken{line.substr(pos, i - pos), "", false});
            }
            std::size_t bold_start = i;
            i += 2;
            std::size_t bold_end = line.find("**", i);
            if (bold_end == std::string::npos) {
                bold_end = len;
            } else {
                bold_end += 2;
            }
            tokens.push_back(SyntaxToken{line.substr(bold_start, bold_end - bold_start), Color::BOLD, false});
            pos = i = bold_end;
        }
        // Check for italic _text_
        else if (line[i] == '_') {
            if (i > pos) {
                tokens.push_back(SyntaxToken{line.substr(pos, i - pos), "", false});
            }
            std::size_t italic_start = i;
            ++i;
            std::size_t italic_end = line.find('_', i);
            if (italic_end == std::string::npos) {
                italic_end = len;
            } else {
                ++italic_end;
            }
            tokens.push_back(SyntaxToken{line.substr(italic_start, italic_end - italic_start), Color::ITALIC, false});
            pos = i = italic_end;
        }
        // Check for [text](url)
        else if (line[i] == '[') {
            if (i > pos) {
                tokens.push_back(SyntaxToken{line.substr(pos, i - pos), "", false});
            }
            std::size_t link_start = i;
            std::size_t bracket_end = line.find(']', i);
            if (bracket_end != std::string::npos && bracket_end + 2 < len && line[bracket_end + 1] == '(') {
                std::size_t paren_end = line.find(')', bracket_end + 2);
                if (paren_end != std::string::npos) {
                    ++paren_end;
                    tokens.push_back(SyntaxToken{line.substr(link_start, paren_end - link_start), Color::CYAN, false});
                    pos = i = paren_end;
                } else {
                    ++i;
                }
            } else {
                ++i;
            }
        }
        // Check for image ![alt](url)
        else if (line[i] == '!' && i + 1 < len && line[i + 1] == '[') {
            if (i > pos) {
                tokens.push_back(SyntaxToken{line.substr(pos, i - pos), "", false});
            }
            std::size_t img_start = i;
            std::size_t bracket_end = line.find(']', img_start + 2);
            if (bracket_end != std::string::npos && bracket_end + 2 < len && line[bracket_end + 1] == '(') {
                std::size_t paren_end = line.find(')', bracket_end + 2);
                if (paren_end != std::string::npos) {
                    ++paren_end;
                    tokens.push_back(SyntaxToken{line.substr(img_start, paren_end - img_start), Color::CYAN, false});
                    pos = i = paren_end;
                } else {
                    ++i;
                }
            } else {
                ++i;
            }
        }
        else {
            ++i;
        }
    }

    // Add remaining text
    if (pos < len) {
        tokens.push_back(SyntaxToken{line.substr(pos), "", false});
    }

    // Add leading whitespace if any
    if (content_start > 0 && tokens.empty()) {
        tokens.push_back(SyntaxToken{line.substr(0, content_start), "", false});
        if (content_start < len) {
            tokens.push_back(SyntaxToken{line.substr(content_start), "", false});
        }
    }

    if (tokens.empty()) {
        tokens.push_back(SyntaxToken{line, "", false});
    }
    return tokens;
}

// Highlight Python code
std::vector<SyntaxToken> highlight_python(const std::string& line) {
    std::vector<SyntaxToken> tokens;

    // String literals (triple quotes)
    if (line.find("\"\"\"") != std::string::npos || line.find("'''") != std::string::npos) {
        tokens.push_back(SyntaxToken{line, Color::YELLOW, false});
        return tokens;
    }

    // String literals
    std::size_t str_start = line.find('"');
    std::size_t single_start = line.find('\'');
    if (single_start < str_start) str_start = single_start;

    std::size_t pos = 0;
    while (str_start != std::string::npos) {
        if (str_start > pos) {
            tokens.push_back(SyntaxToken{line.substr(pos, str_start - pos), "", false});
        }
        char quote_char = line[str_start];
        std::size_t search_from = str_start + 1;
        while (search_from < line.length()) {
            if (line[search_from] == '\\' && search_from + 1 < line.length()) {
                search_from += 2;
            } else if (line[search_from] == quote_char) {
                break;
            } else {
                ++search_from;
            }
        }
        std::size_t str_end = (search_from < line.length()) ? search_from + 1 : line.length();
        tokens.push_back(SyntaxToken{line.substr(str_start, str_end - str_start), Color::YELLOW, false});
        pos = str_end;
        str_start = line.find(quote_char, search_from);
    }

    // Single-line comment
    auto comment_pos = line.find("#");
    if (comment_pos != std::string::npos) {
        if (comment_pos > pos) {
            tokens.push_back(SyntaxToken{line.substr(pos, comment_pos - pos), "", false});
        }
        tokens.push_back(SyntaxToken{line.substr(comment_pos), Color::DIM, false});
        return tokens;
    }

    // Remaining text with keyword highlighting
    if (pos < line.length()) {
        std::string remaining = line.substr(pos);

        const char* keywords[] = {
            "def", "class", "if", "elif", "else", "while", "for", "in", "try",
            "except", "finally", "with", "as", "import", "from", "return", "yield",
            "raise", "pass", "break", "continue", "lambda", "and", "or", "not",
            "is", "global", "nonlocal", "assert", "del", "async", "await",
            "True", "False", "None"
        };

        std::string result;
        std::size_t rpos = 0;
        while (rpos < remaining.length()) {
            // Check for keyword
            bool is_keyword = false;
            for (const char* kw : keywords) {
                std::size_t klen = strlen(kw);
                if (remaining.compare(rpos, klen, kw) == 0) {
                    char prev = (rpos > 0) ? remaining[rpos - 1] : ' ';
                    char next = (rpos + klen < remaining.length()) ? remaining[rpos + klen] : ' ';
                    if (!std::isalnum(prev) && !std::isalnum(next) && prev != '_' && next != '_') {
                        if (rpos < remaining.length() && !result.empty()) {
                            result += Color::RESET;
                        }
                        result += Color::BLUE;
                        result += Color::BOLD;
                        result += kw;
                        result += Color::RESET;
                        rpos += klen;
                        is_keyword = true;
                        break;
                    }
                }
            }
            if (!is_keyword) {
                result += remaining[rpos];
                ++rpos;
            }
        }

        tokens.push_back(SyntaxToken{result, "", false});
    }

    if (tokens.empty()) {
        tokens.push_back(SyntaxToken{line, "", false});
    }
    return tokens;
}

// Highlight JSON
std::vector<SyntaxToken> highlight_json(const std::string& line) {
    std::vector<SyntaxToken> tokens;
    std::size_t pos = 0;
    std::size_t len = line.length();

    while (pos < len) {
        // String keys and values (in quotes)
        if (line[pos] == '"') {
            std::size_t str_start = pos;
            std::size_t search_from = pos + 1;

            // Find end of string (handle escaped characters)
            while (search_from < len) {
                if (line[search_from] == '\\' && search_from + 1 < len) {
                    search_from += 2;
                } else if (line[search_from] == '"') {
                    break;
                } else {
                    ++search_from;
                }
            }
            std::size_t str_end = (search_from < len) ? search_from + 1 : len;

            // Check if this looks like a JSON key (followed by :)
            bool is_key = false;
            std::size_t after_quote = str_end;
            while (after_quote < len && (line[after_quote] == ' ' || line[after_quote] == '\t')) {
                ++after_quote;
            }
            if (after_quote < len && line[after_quote] == ':') {
                is_key = true;
            }

            // Keys get purple, string values get yellow
            std::string color = is_key ? Color::MAGENTA : Color::YELLOW;
            tokens.push_back(SyntaxToken{line.substr(str_start, str_end - str_start), color, false});
            pos = str_end;
        }
        // Numbers
        else if (std::isdigit(line[pos]) || (line[pos] == '-' && pos + 1 < len && std::isdigit(line[pos + 1]))) {
            std::size_t num_start = pos;
            if (line[pos] == '-') ++pos;
            while (pos < len && (std::isdigit(line[pos]) || line[pos] == '.' || line[pos] == 'e' || line[pos] == 'E' ||
                                  (line[pos] == '-' && (line[pos-1] == 'e' || line[pos-1] == 'E')) ||
                                  (line[pos] == '+' && (line[pos-1] == 'e' || line[pos-1] == 'E')))) {
                ++pos;
            }
            tokens.push_back(SyntaxToken{line.substr(num_start, pos - num_start), Color::CYAN, false});
        }
        // Boolean and null
        else if (line.compare(pos, 4, "true") == 0 || line.compare(pos, 5, "false") == 0 ||
                 line.compare(pos, 4, "null") == 0 || line.compare(pos, 4, "null") == 0) {
            std::size_t kw_start = pos;
            std::size_t kw_len = 4;
            if (pos + 5 <= len && line.compare(pos, 5, "false") == 0) {
                kw_len = 5;
            }
            tokens.push_back(SyntaxToken{line.substr(kw_start, kw_len), Color::GREEN, true});
            pos += kw_len;
        }
        // Brackets and braces
        else if (line[pos] == '{' || line[pos] == '}' || line[pos] == '[' || line[pos] == ']') {
            tokens.push_back(SyntaxToken{std::string(1, line[pos]), Color::BRIGHT_RED, true});
            ++pos;
        }
        // Whitespace
        else {
            tokens.push_back(SyntaxToken{std::string(1, line[pos]), "", false});
            ++pos;
        }
    }

    if (tokens.empty()) {
        tokens.push_back(SyntaxToken{line, "", false});
    }
    return tokens;
}

std::vector<SyntaxToken> highlight_line(
    const std::string& line,
    const SyntaxDefinition& syntax,
    bool in_multiline_comment
) {
    if (syntax.name == "cpp") {
        return highlight_cpp(line);
    } else if (syntax.name == "python") {
        return highlight_python(line);
    } else if (syntax.name == "markdown") {
        return highlight_markdown(line);
    } else if (syntax.name == "json") {
        return highlight_json(line);
    }

    // Default: no highlighting
    std::vector<SyntaxToken> tokens;
    tokens.push_back(SyntaxToken{line, "", false});
    return tokens;
}

}  // namespace fastcat
