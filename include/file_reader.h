#ifndef FASTCAT_FILE_READER_H
#define FASTCAT_FILE_READER_H

#include <string>
#include <optional>
#include <variant>
#include <cstddef>

namespace fastcat {

// File size category for appropriate handling strategy
enum class FileSize {
    Small,      // < 1MB: Can load into memory
    Medium,     // 1MB - 100MB: Stream with caching
    Large,      // > 100MB: Pure streaming required
};

// File metadata
struct FileInfo {
    std::string path;
    std::uintmax_t size;
    FileSize size_category;
};

// Result of file reading operation
struct ReadResult {
    std::string line;
    std::size_t line_number;
    bool is_eof;
};

// Abstract file reader interface
class IFileReader {
public:
    virtual ~IFileReader() = default;
    virtual std::optional<ReadResult> read_line() = 0;
    virtual bool seek(std::size_t line_number) = 0;
    virtual FileInfo info() const = 0;
    virtual bool is_large() const = 0;
    virtual void rewind() = 0;
};

// Create appropriate reader based on file size
std::unique_ptr<IFileReader> create_file_reader(const std::string& path);

}  // namespace fastcat

#endif  // FASTCAT_FILE_READER_H
