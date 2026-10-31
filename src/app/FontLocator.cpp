#include "app/FontLocator.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <string_view>
#include <system_error>
#include <vector>

namespace bb::app
{
namespace
{
[[nodiscard]] bool IsFile(const std::filesystem::path& path)
{
    std::error_code error;
    return std::filesystem::is_regular_file(path, error);
}

[[nodiscard]] std::optional<std::filesystem::path> FindBundledFont(const std::filesystem::path& fontsDirectory)
{
    std::error_code error;
    if (!std::filesystem::is_directory(fontsDirectory, error))
    {
        return std::nullopt;
    }

    std::vector<std::filesystem::path> fonts;
    for (std::filesystem::directory_iterator it(fontsDirectory, error); !error && it != std::filesystem::directory_iterator{}; it.increment(error))
    {
        const std::filesystem::path extension = it->path().extension();
        if (extension == ".ttf" || extension == ".otf" || extension == ".ttc")
        {
            fonts.push_back(it->path());
        }
    }
    if (fonts.empty())
    {
        return std::nullopt;
    }

    std::ranges::sort(fonts);
    return fonts.front();
}

// C++ Concept: Platform differences behind a preprocessor switch, hidden inside one function
// Only this function knows which operating system it runs on; callers just receive a list of paths. Keeping
// `#if` blocks small and local like this stops platform ifdefs from spreading through the code base.
[[nodiscard]] std::vector<std::filesystem::path> SystemFontCandidates()
{
    std::vector<std::filesystem::path> candidates;

#if defined(_WIN32)
    const char* windowsDirectory = std::getenv("WINDIR");
    const std::filesystem::path fonts = std::filesystem::path(windowsDirectory != nullptr ? windowsDirectory : "C:\\Windows") / "Fonts";
    for (const std::string_view name : std::array<std::string_view, 5>{"segoeui.ttf", "arial.ttf", "tahoma.ttf", "verdana.ttf", "calibri.ttf"})
    {
        candidates.push_back(fonts / name);
    }
#elif defined(__APPLE__)
    for (const std::string_view path : std::array<std::string_view, 4>{"/System/Library/Fonts/Supplemental/Arial.ttf", "/System/Library/Fonts/Supplemental/Verdana.ttf", "/Library/Fonts/Arial.ttf", "/System/Library/Fonts/Helvetica.ttc"})
    {
        candidates.emplace_back(path);
    }
#else
    for (const std::string_view path : std::array<std::string_view, 6>{"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/TTF/DejaVuSans.ttf", "/usr/share/fonts/dejavu/DejaVuSans.ttf", "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", "/usr/share/fonts/liberation/LiberationSans-Regular.ttf", "/usr/share/fonts/noto/NotoSans-Regular.ttf"})
    {
        candidates.emplace_back(path);
    }
#endif

    return candidates;
}
} // namespace

std::optional<std::filesystem::path> FindFontFile(const AssetPaths& assets)
{
    if (std::optional<std::filesystem::path> bundled = FindBundledFont(assets.Fonts()))
    {
        return bundled;
    }

    for (const std::filesystem::path& candidate : SystemFontCandidates())
    {
        if (IsFile(candidate))
        {
            return candidate;
        }
    }
    return std::nullopt;
}
} // namespace bb::app
