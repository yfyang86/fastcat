# fastcat

A modern `cat` replacement with syntax highlighting, CSV formatting, and pipeline support.

## Build

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_COMPILER=clang++
make -j4
```

## Usage

```bash
fastcat [OPTIONS] [FILES...]
```

## Options

| Option | Short | Description |
|--------|-------|-------------|
| `--help` | `-h` | Show help message |
| `--theme` | | Enable vim-like theme (bold, colors) |
| `--syntax <type>` | `-s` | Enable syntax highlighting (cpp, py, md, json, csv) |
| `--align-csv` | | Align and display CSV as a table |
| `--rainbowcsv` | | Display CSV with rainbow-colored columns (256-color) |
| `--pager` | `-p` | Use pager for output (less-like mode) |
| `--no-pager` | | Never use pager |
| `--linenumber` | `-n` | Show line numbers |
| `-e` | | Read from stdin (pipeline mode) |

## Option Dependencies

The following options implicitly enable or affect other behaviors:

| Primary Option | Implicit Effect |
|----------------|-----------------|
| `--align-csv` | Sets `--syntax csv` automatically if CSV is detected |
| `--rainbowcsv` | Enables CSV table formatting with colored columns |
| `-e` (stdin) | Supports all other options for piped input |
| `--theme` | Applies bold/color theme to syntax highlighting |

## Examples

### Basic File Viewing

```bash
# View a file
fastcat file.txt

# With line numbers
fastcat -n file.txt

# With vim-like theme
fastcat --theme file.txt
```

### Syntax Highlighting

```bash
# C++ with syntax highlighting
fastcat --syntax cpp file.cpp

# Python with line numbers
fastcat -n --syntax py script.py

# Markdown preview
fastcat --syntax md readme.md

# JSON with syntax highlighting
fastcat --syntax json config.json

# JSON from pipeline
echo '{"name": "test"}' | fastcat --syntax json -e
```

### CSV Formatting

```bash
# Align CSV as table
fastcat --align-csv data.csv

# Rainbow CSV with colored columns
fastcat --rainbowcsv data.csv
```

### Pipeline Mode

```bash
# Pipe code with syntax highlighting
echo '#include <iostream>' | fastcat --syntax cpp -e

# Full pipeline with line numbers and theme
echo 'def hello():\n    print("world")' | fastcat -n --theme --syntax py -e

# CSV from pipeline
echo 'name,age,city\nAlice,30,NYC' | fastcat --rainbowcsv -e
```

### Large File Handling

For files larger than 1MB, fastcat automatically uses streaming mode:

```bash
# Auto-pager for large files (when output is terminal)
fastcat large_file.log

# Force pager
fastcat --pager very_large_file.txt

# Disable pager
fastcat --no-pager large_file.txt
```

## Feature Summary

| Feature | Description |
|---------|-------------|
| Syntax Highlighting | C++, Python, Markdown, JSON support |
| CSV Formatting | Table alignment with column widths |
| Rainbow CSV | 256-color column highlighting |
| Line Numbers | Optional per-line numbering |
| Theme Support | Vim-like color scheme |
| Pipeline Mode | Read from stdin with `-e` |
| Large File Support | Streaming for files > 1MB |
| Auto Pager | Less-like mode for large files |

## Architecture

```
fastcat/
├── CMakeLists.txt
├── Readme.md
├── include/
│   ├── args.h          # CLI argument parsing
│   ├── file_reader.h   # Streaming/memory-mapped reader
│   ├── syntax_highlight.h  # Syntax engine
│   ├── csv_formatter.h # CSV parsing & formatting
│   ├── theme.h         # Color themes
│   └── pager.h         # Pagination
└── src/
    ├── main.cpp
    ├── args.cpp
    ├── file_reader.cpp
    ├── syntax_highlight.cpp
    ├── csv_formatter.cpp
    ├── theme.cpp
    └── pager.cpp
```

## License

Apache-2.0

Martingalefan <yfyang86>

## Logs

- 20260201: Initial the project


