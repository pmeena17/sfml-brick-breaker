#pragma once

#include <filesystem>
#include <optional>

#include "app/AssetLocator.hpp"

namespace bb::app
{
// Finds a TrueType/OpenType font to draw text with.
//
// SFML (like most graphics libraries) has no built-in font, so the game needs a font file. Search order:
//   1. any .ttf/.otf/.ttc in assets/fonts (drop your favourite font there to override everything),
//   2. well-known system fonts (Segoe UI/Arial on Windows, Arial/Helvetica on macOS, DejaVu/Liberation on Linux).
// Windows ships with Segoe UI and Arial, so the game works there with no extra files.
[[nodiscard]] std::optional<std::filesystem::path> FindFontFile(const AssetPaths& assets);
} // namespace bb::app
