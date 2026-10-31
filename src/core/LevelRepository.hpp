#pragma once

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

#include "core/LevelData.hpp"

namespace bb::core
{
// A level file that could not be loaded, and why.
struct LevelLoadIssue
{
    std::filesystem::path file;
    std::string message;
};

// The ordered collection of all playable levels.
//
// Levels are DISCOVERED, not registered: LoadFromDirectory() loads every "*.level" file in a folder,
// sorted by file name. To add a level you drop a new file in assets/levels (e.g. level_06.level) - no
// code changes, no recompilation, and the menu grows a button for it automatically.
//
// A broken file never prevents the game from starting: it is skipped and recorded in Issues() so the
// application can report it.
//
// C++ Concept: The Rule of Zero
// This class owns two std::vectors, which already know how to copy, move and
// destroy themselves. So LevelRepository declares NO destructor, copy or move operations at all, and the
// compiler-generated ones are correct. Only write those special members when you manage a raw resource
// (memory, file handle, socket) yourself - and then wrap that resource in its own small RAII class so
// the rest of your code can follow the Rule of Zero again.
class LevelRepository
{
public:
    // Loads every "*.level" file in `directory`. Missing directory => an empty repository plus an issue.
    [[nodiscard]] static LevelRepository LoadFromDirectory(const std::filesystem::path& directory);

    // Builds a repository from levels created in code (tests, procedural levels, other file formats).
    [[nodiscard]] static LevelRepository FromLevels(std::vector<LevelData> levels);

    [[nodiscard]] std::size_t Count() const noexcept
    {
        return m_levels.size();
    }

    [[nodiscard]] bool Empty() const noexcept
    {
        return m_levels.empty();
    }

    // Throws std::out_of_range for an invalid index.
    [[nodiscard]] const LevelData& At(std::size_t index) const;

    [[nodiscard]] bool HasNext(std::size_t index) const noexcept
    {
        return index + 1 < m_levels.size();
    }

    // C++ Concept: std::span (C++20) - a non-owning view over contiguous elements
    // Returning `const std::vector<LevelData>&` would tie every caller to "it is a std::vector". A
    // std::span<const LevelData> says only "here are N contiguous, read-only levels": it works whether the
    // storage is a vector, an array or a C buffer, carries the size with the pointer (unlike `T*` + `int n`
    // in the original C code), and cannot be used to add or remove elements. Like string_view it does
    // not own the data - don't keep it past the lifetime of the container it views.
    [[nodiscard]] std::span<const LevelData> Levels() const noexcept
    {
        return m_levels;
    }

    [[nodiscard]] const std::vector<LevelLoadIssue>& Issues() const noexcept
    {
        return m_issues;
    }

private:
    std::vector<LevelData> m_levels;
    std::vector<LevelLoadIssue> m_issues;
};
} // namespace bb::core
