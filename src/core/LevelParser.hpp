#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>

#include "core/LevelData.hpp"

namespace bb::core
{
// Thrown when a level file is malformed. The message always has the form "file:line: what went wrong",
// so a level designer can jump straight to the offending line.
//
// C++ Concept: Custom exception types derived from std::runtime_error
// Exceptions are the C++ way to report errors that the current function cannot handle itself (here:
// "this file is not a valid level"). Deriving from a standard exception means generic code can catch
// it as `const std::exception&` and call what(), while code that cares can catch the specific type
// and read extra data (Line()). Throw by value, catch by const reference.
// Guideline: use exceptions for exceptional, non-local failures (bad input file); use std::optional
// or a return value for outcomes that are expected and routine (e.g. ParsePowerUpId returning nothing).
class LevelParseError : public std::runtime_error
{
public:
    LevelParseError(std::string_view source, int line, std::string_view message);

    // 1-based line number of the problem, or 0 if it is not tied to a single line.
    [[nodiscard]] int Line() const noexcept
    {
        return m_line;
    }

private:
    int m_line;
};

// Parses the text of a level file. `sourceName` is only used in error messages.
// Throws LevelParseError on any problem. The returned level is fully validated: every brick is inside
// the playfield and the level has a name and at least one brick.
[[nodiscard]] LevelData ParseLevel(std::string_view text, std::string_view sourceName);

// Reads and parses a level file. Throws LevelParseError for bad content, std::runtime_error if the
// file cannot be read.
[[nodiscard]] LevelData LoadLevelFile(const std::filesystem::path& path);
} // namespace bb::core
