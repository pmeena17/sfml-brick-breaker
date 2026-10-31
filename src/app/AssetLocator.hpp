#pragma once

#include <filesystem>
#include <optional>

namespace bb::app
{
// Where the game's data files live.
struct AssetPaths
{
    std::filesystem::path root;

    [[nodiscard]] std::filesystem::path Levels() const
    {
        return root / "levels";
    }

    [[nodiscard]] std::filesystem::path Fonts() const
    {
        return root / "fonts";
    }
};

// Finds the "assets" folder, trying in order:
//   1. the folder named by the BRICK_BREAKER_ASSETS environment variable,
//   2. "assets" next to the executable (the layout of an installed/packaged game),
//   3. "assets" in the current working directory,
//   4. the source-tree folder that was recorded when the game was built (convenient during development).
// A candidate counts only if it contains a "levels" sub-folder. Returns std::nullopt if none does.
//
// The original program relied on whatever the graphics library's working directory happened to be and
// had nothing to load, so this was never a problem; once data lives in files, finding them reliably is.
[[nodiscard]] std::optional<AssetPaths> LocateAssets(const std::filesystem::path& executablePath);
} // namespace bb::app
