#include "core/LevelRepository.hpp"

#include <algorithm>
#include <exception>
#include <stdexcept>
#include <system_error>
#include <utility>

#include "core/LevelParser.hpp"

namespace bb::core
{
LevelRepository LevelRepository::LoadFromDirectory(const std::filesystem::path& directory)
{
    LevelRepository repository;

    // C++ Concept: std::filesystem (C++17) and the std::error_code overloads
    // <filesystem> gives portable path handling (`/` joins path parts on Windows, Linux and macOS alike,
    // no manual "\\" vs "/") and directory iteration. Most functions come in two flavours: one that
    // THROWS std::filesystem::filesystem_error, and one that takes a `std::error_code&` and reports
    // failure through it. When "the folder does not exist" is an expected situation (as here) use the
    // error_code flavour and handle it, rather than wrapping everything in try/catch.
    std::error_code error;
    if (!std::filesystem::is_directory(directory, error))
    {
        repository.m_issues.push_back({directory, "level directory not found"});
        return repository;
    }

    std::vector<std::filesystem::path> files;
    for (std::filesystem::directory_iterator it(directory, error); !error && it != std::filesystem::directory_iterator{}; it.increment(error))
    {
        std::error_code entryError;
        if (it->is_regular_file(entryError) && it->path().extension() == ".level")
        {
            files.push_back(it->path());
        }
    }
    if (error)
    {
        repository.m_issues.push_back({directory, "cannot read level directory: " + error.message()});
    }

    // C++ Concept: std::ranges::sort - sorting without iterator pairs
    // Directory iteration order is unspecified (it differs between Windows and Linux), so sort by file
    // name to make the level order deterministic: level_01, level_02, ... (use zero-padded numbers).
    std::ranges::sort(files);

    for (const std::filesystem::path& file : files)
    {
        // One bad file must not take the whole game down: catch, record, carry on.
        // We catch `const std::exception&` (the common base of LevelParseError and I/O errors) by
        // reference to avoid slicing the exception object.
        try
        {
            repository.m_levels.push_back(LoadLevelFile(file));
        }
        catch (const std::exception& exception)
        {
            repository.m_issues.push_back({file, exception.what()});
        }
    }

    return repository;
}

LevelRepository LevelRepository::FromLevels(std::vector<LevelData> levels)
{
    LevelRepository repository;
    // C++ Concept: Sink parameters - pass by value, then std::move
    // When a function is going to KEEP its argument, take it by value and move it into place. The caller
    // chooses: pass an lvalue (one copy is made) or `std::move(x)` / a temporary (no copy, just a cheap
    // move). std::move does not move anything by itself - it is a cast that says "this object may be
    // pillaged"; the actual stealing happens in the move constructor/assignment.
    // Never use an object after it has been moved from (except to assign to it or destroy it).
    repository.m_levels = std::move(levels);
    return repository;
}

const LevelData& LevelRepository::At(std::size_t index) const
{
    if (index >= m_levels.size())
    {
        throw std::out_of_range("level index out of range");
    }
    return m_levels[index];
}
} // namespace bb::core
