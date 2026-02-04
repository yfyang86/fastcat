#include "file_reader.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <memory>

namespace fastcat {

namespace fs = std::filesystem;

FileInfo get_file_info(const std::string& path) {
    FileInfo info;
    info.path = path;

    try {
        info.size = fs::file_size(path);
    } catch (const fs::filesystem_error&) {
        info.size = 0;
    }

    if (info.size < 1024 * 1024) {
        info.size_category = FileSize::Small;
    } else if (info.size < 100 * 1024 * 1024) {
        info.size_category = FileSize::Medium;
    } else {
        info.size_category = FileSize::Large;
    }

    return info;
}

// Streaming reader for large files
class StreamingFileReader : public IFileReader {
public:
    explicit StreamingFileReader(const std::string& path)
        : file_(path), line_number_(0) {
        if (!file_.is_open()) {
            std::cerr << "Warning: Cannot open file: " << path << "\n";
        }
    }

    std::optional<ReadResult> read_line() override {
        std::string line;
        if (std::getline(file_, line)) {
            ++line_number_;
            return ReadResult{line, line_number_, false};
        }
        return ReadResult{"", line_number_, true};
    }

    bool seek(std::size_t line_number) override {
        // For streaming reader, seeking backwards is expensive
        // We can only seek forward efficiently
        if (line_number <= line_number_) {
            // Rewind and read to the target line
            file_.clear();
            file_.seekg(0);
            line_number_ = 0;
        }

        std::string line;
        while (line_number_ < line_number && std::getline(file_, line)) {
            ++line_number_;
        }

        return file_.good() || file_.eof();
    }

    FileInfo info() const override {
        return get_file_info(path_);
    }

    bool is_large() const override {
        return info().size_category != FileSize::Small;
    }

    void rewind() override {
        file_.clear();
        file_.seekg(0);
        line_number_ = 0;
    }

private:
    std::ifstream file_;
    std::size_t line_number_;
    std::string path_;
};

// Memory-mapped reader for small files (faster random access)
class MemoryMappedReader : public IFileReader {
public:
    explicit MemoryMappedReader(const std::string& path) : line_number_(0) {
        file_.open(path, std::ios::binary);
        if (file_.is_open()) {
            file_size_ = fs::file_size(path);
            mapping_ = std::make_unique<char[]>(file_size_);
            file_.read(mapping_.get(), file_size_);
        }
    }

    std::optional<ReadResult> read_line() override {
        if (offset_ >= file_size_) {
            return ReadResult{"", line_number_, true};
        }

        std::size_t start = offset_;
        while (offset_ < file_size_ && mapping_[offset_] != '\n') {
            ++offset_;
        }

        std::string line(mapping_.get() + start, offset_ - start);
        if (offset_ < file_size_) ++offset_;  // Skip newline
        ++line_number_;

        return ReadResult{line, line_number_, false};
    }

    bool seek(std::size_t line_number) override {
        if (line_number == 0) {
            offset_ = 0;
            line_number_ = 0;
            return true;
        }

        if (line_number < line_number_) {
            // Rewind and find the line
            offset_ = 0;
            line_number_ = 0;
        }

        while (line_number_ < line_number) {
            auto result = read_line();
            if (result && result->is_eof) {
                return false;
            }
        }

        return true;
    }

    FileInfo info() const override {
        return get_file_info(path_);
    }

    bool is_large() const override {
        return false;  // Memory-mapped is only used for small files
    }

    void rewind() override {
        offset_ = 0;
        line_number_ = 0;
    }

private:
    std::ifstream file_;
    std::unique_ptr<char[]> mapping_;
    std::size_t file_size_;
    std::size_t offset_;
    std::size_t line_number_;
    std::string path_;
};

std::unique_ptr<IFileReader> create_file_reader(const std::string& path) {
    if (path == "-") {
        // Stdin - always use streaming
        return std::make_unique<StreamingFileReader>("/dev/stdin");
    }

    auto info = get_file_info(path);

    switch (info.size_category) {
        case FileSize::Small:
            return std::make_unique<MemoryMappedReader>(path);
        case FileSize::Medium:
        case FileSize::Large:
        default:
            return std::make_unique<StreamingFileReader>(path);
    }
}

}  // namespace fastcat
