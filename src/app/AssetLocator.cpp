#include "app/AssetLocator.hpp"

#include <cstdlib>
#include <system_error>
#include <vector>

namespace bb::app
{
namespace
{
[[nodiscard]] bool LooksLikeAssetsFolder(const std::filesystem::path& candidate)
{
    std::error_code error;
    return std::filesystem::is_directory(candidate / "levels", error);
}
} // namespace

std::optional<AssetPaths> LocateAssets(const std::filesystem::path& executablePath)
{
    std::vector<std::filesystem::path> candidates;

    if (const char* fromEnvironment = std::getenv("BRICK_BREAKER_ASSETS"); fromEnvironment != nullptr && fromEnvironment[0] != '\0')
    {
        candidates.emplace_back(fromEnvironment);
    }

    if (!executablePath.empty())
    {
        candidates.push_back(executablePath.parent_path() / "assets");
    }

    std::error_code error;
    const std::filesystem::path workingDirectory = std::filesystem::current_path(error);
    if (!error)
    {
        candidates.push_back(workingDirectory / "assets");
    }

    // C++ Concept: Conditional compilation driven by the build system
    // CMake defines BB_DEFAULT_ASSETS_DIR (see src/app/CMakeLists.txt) as an absolute path into the source tree.
    // `#ifdef` keeps this fallback out of builds that do not define it. Use the preprocessor ONLY for things the
    // language cannot express (platform differences, build-time switches); for everything else prefer
    // constexpr, templates and normal functions, which are type-checked.
#ifdef BB_DEFAULT_ASSETS_DIR
    candidates.emplace_back(BB_DEFAULT_ASSETS_DIR);
#endif

    for (const std::filesystem::path& candidate : candidates)
    {
        if (LooksLikeAssetsFolder(candidate))
        {
            return AssetPaths{candidate};
        }
    }
    return std::nullopt;
}
} // namespace bb::app
