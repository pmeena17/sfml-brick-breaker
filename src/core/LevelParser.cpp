#include "core/LevelParser.hpp"

#include <charconv>
#include <concepts>
#include <cstddef>
#include <fstream>
#include <optional>
#include <sstream>
#include <system_error>
#include <vector>

#include "core/Config.hpp"
#include "core/PowerUp.hpp"
#include "core/Rect.hpp"
#include "core/Vec2.hpp"

namespace bb::core
{
namespace
{
std::string BuildMessage(std::string_view source, int line, std::string_view message)
{
    std::string text(source);
    if (line > 0)
    {
        text += ':';
        text += std::to_string(line);
    }
    text += ": ";
    text += message;
    return text;
}
} // namespace

LevelParseError::LevelParseError(std::string_view source, int line, std::string_view message) : std::runtime_error(BuildMessage(source, line, message)), m_line(line)
{
}

namespace
{
// Everything a helper needs to report a problem at the right place.
struct ParseContext
{
    std::string_view source;
    int line{0};

    // C++ Concept: [[noreturn]]
    // Tells the compiler this function never returns normally (it always throws). The compiler can then
    // stop warning about "control reaches end of non-void function" after a call to Fail(), and the
    // reader knows `if (bad) context.Fail(...)` really does abandon the current path.
    [[noreturn]] void Fail(std::string_view message) const
    {
        throw LevelParseError(source, line, message);
    }
};

// C++ Concept: Constrained templates with `concepts` (C++20), std::from_chars and structured bindings
//   * `template <std::integral T>` means "T must be an integer type". Passing a float or a std::string
//     is a clear one-line compile error ("constraints not satisfied") instead of 40 lines of template
//     noise. Concepts are named, reusable requirements on template parameters.
//   * std::from_chars converts text to a number without allocating, without locale surprises (the
//     C function atoi/stod use the current locale - "1,5" vs "1.5") and WITHOUT exceptions or the
//     silent failure of atoi (which returns 0 for garbage). It reports success through an error code.
//   * `const auto [pointer, error] = ...` is a structured binding (C++17): it unpacks the returned
//     struct {ptr, ec} into two named variables in one line.
// We also demand that the WHOLE token was consumed (`pointer == last`), so "12abc" is rejected.
template <std::integral T>
[[nodiscard]] std::optional<T> ParseNumber(std::string_view text) noexcept
{
    T value{};
    const char* const first = text.data();
    const char* const last = first + text.size();
    const auto [pointer, error] = std::from_chars(first, last, value);
    if (error != std::errc{} || pointer != last)
    {
        return std::nullopt;
    }
    return value;
}

constexpr std::string_view kWhitespace = " \t\r\n";

// C++ Concept: std::string_view as a non-owning "window" into text you already have
// The parser walks over the file contents without ever copying them: Trim(), Tokenize() and
// substr() all return string_views pointing into the ORIGINAL buffer. That is fast, but it comes
// with the one rule you must remember: a string_view must not outlive the string it points into
// (never return a view of a local std::string, never store a view of a temporary).
[[nodiscard]] std::string_view Trim(std::string_view text) noexcept
{
    const std::size_t first = text.find_first_not_of(kWhitespace);
    if (first == std::string_view::npos)
    {
        return {};
    }
    const std::size_t last = text.find_last_not_of(kWhitespace);
    return text.substr(first, last - first + 1);
}

[[nodiscard]] std::vector<std::string_view> Tokenize(std::string_view line)
{
    std::vector<std::string_view> tokens;
    while (true)
    {
        const std::size_t start = line.find_first_not_of(kWhitespace);
        if (start == std::string_view::npos)
        {
            break;
        }
        line.remove_prefix(start);

        const std::size_t end = line.find_first_of(kWhitespace);
        tokens.push_back(line.substr(0, end)); // npos means "to the end"
        if (end == std::string_view::npos)
        {
            break;
        }
        line.remove_prefix(end);
    }
    return tokens;
}

[[nodiscard]] int ParsePositiveInt(const ParseContext& context, std::string_view text, std::string_view what)
{
    const std::optional<int> value = ParseNumber<int>(text);
    if (!value || *value <= 0)
    {
        context.Fail(std::string(what) + " must be a positive whole number, got '" + std::string(text) + "'");
    }
    return *value;
}

[[nodiscard]] std::string ListPowerUpIds()
{
    std::string ids;
    for (const PowerUpDefinition& definition : kPowerUpDefinitions)
    {
        if (!ids.empty())
        {
            ids += ", ";
        }
        ids += definition.id;
    }
    return ids;
}

void ValidateBrickBounds(const ParseContext& context, const Rect& bounds)
{
    const Rect& field = config::kPlayfield;
    if (bounds.Left() < field.Left() || bounds.Right() > field.Right() || bounds.Top() < field.Top())
    {
        const std::string where = "(" + std::to_string(static_cast<int>(bounds.Center().x)) + ", " + std::to_string(static_cast<int>(bounds.Center().y)) + ")";
        context.Fail("the brick at " + where + " sticks out of the playfield (x " + std::to_string(static_cast<int>(field.Left())) + ".." + std::to_string(static_cast<int>(field.Right())) + ", y from " + std::to_string(static_cast<int>(field.Top())) + ")");
    }

    // Keep one ball-diameter of free space above the paddle so a level can always be finished.
    const float lowestAllowedBottom = config::paddle::kCenterY - config::paddle::kHeight * 0.5f - 2.0f * config::ball::kRadius;
    if (bounds.Bottom() > lowestAllowedBottom)
    {
        context.Fail("a brick is too low: its bottom edge must be at y <= " + std::to_string(static_cast<int>(lowestAllowedBottom)) + " so the ball can pass above the paddle");
    }
}

void ParseBrickDirective(const ParseContext& context, const std::vector<std::string_view>& tokens, Vec2 defaultSize, LevelData& level)
{
    if (tokens.size() < 3)
    {
        context.Fail("'brick' needs a position: brick <x> <y> [w=N] [h=N] [hp=N] [power=ID]");
    }

    const std::optional<int> x = ParseNumber<int>(tokens[1]);
    const std::optional<int> y = ParseNumber<int>(tokens[2]);
    if (!x || !y)
    {
        context.Fail("the brick position must be two whole numbers, got '" + std::string(tokens[1]) + " " + std::string(tokens[2]) + "'");
    }

    Vec2 size = defaultSize;
    int hitPoints = 1;
    std::optional<PowerUpKind> power;

    for (std::size_t index = 3; index < tokens.size(); ++index)
    {
        const std::string_view option = tokens[index];
        const std::size_t equals = option.find('=');
        if (equals == std::string_view::npos)
        {
            context.Fail("expected key=value but found '" + std::string(option) + "'");
        }

        const std::string_view key = option.substr(0, equals);
        const std::string_view value = option.substr(equals + 1);

        if (key == "w")
        {
            size.x = static_cast<float>(ParsePositiveInt(context, value, "w"));
        }
        else if (key == "h")
        {
            size.y = static_cast<float>(ParsePositiveInt(context, value, "h"));
        }
        else if (key == "hp")
        {
            hitPoints = ParsePositiveInt(context, value, "hp");
            if (hitPoints > config::brick::kMaxHitPoints)
            {
                context.Fail("hp must be between 1 and " + std::to_string(config::brick::kMaxHitPoints) + ", got " + std::to_string(hitPoints));
            }
        }
        else if (key == "power")
        {
            power = ParsePowerUpId(value);
            if (!power)
            {
                context.Fail("unknown power-up '" + std::string(value) + "' (valid: " + ListPowerUpIds() + ")");
            }
        }
        else
        {
            context.Fail("unknown brick option '" + std::string(key) + "' (valid: w, h, hp, power)");
        }
    }

    const Rect bounds = Rect::FromCenter({static_cast<float>(*x), static_cast<float>(*y)}, size);
    ValidateBrickBounds(context, bounds);
    level.bricks.emplace_back(bounds, hitPoints, power);
}
} // namespace

LevelData ParseLevel(std::string_view text, std::string_view sourceName)
{
    // Files saved by Windows Notepad may start with a UTF-8 byte-order mark; skip it so "name" on the
    // first line is still recognised. (std::string_view::starts_with is C++20.)
    if (text.starts_with("\xEF\xBB\xBF"))
    {
        text.remove_prefix(3);
    }

    LevelData level;
    Vec2 defaultBrickSize = config::brick::kDefaultSize;
    bool hasName = false;
    ParseContext context{.source = sourceName, .line = 0};

    while (!text.empty())
    {
        // Peel one line off the front of the text.
        const std::size_t newline = text.find('\n');
        std::string_view line = text.substr(0, newline);
        text = (newline == std::string_view::npos) ? std::string_view{} : text.substr(newline + 1);
        ++context.line;

        line = line.substr(0, line.find('#')); // drop comments
        const std::vector<std::string_view> tokens = Tokenize(line);
        if (tokens.empty())
        {
            continue;
        }

        const std::string_view directive = tokens.front();
        if (directive == "name")
        {
            if (hasName)
            {
                context.Fail("'name' is given more than once");
            }
            const std::size_t directiveOffset = static_cast<std::size_t>(directive.data() - line.data());
            const std::string_view value = Trim(line.substr(directiveOffset + directive.size()));
            if (value.empty())
            {
                context.Fail("'name' needs a value, e.g. name Level 6");
            }
            level.name = std::string(value);
            hasName = true;
        }
        else if (directive == "brick_size")
        {
            if (tokens.size() != 3)
            {
                context.Fail("'brick_size' needs two numbers: brick_size <width> <height>");
            }
            defaultBrickSize = {static_cast<float>(ParsePositiveInt(context, tokens[1], "the brick width")), static_cast<float>(ParsePositiveInt(context, tokens[2], "the brick height"))};
        }
        else if (directive == "brick")
        {
            ParseBrickDirective(context, tokens, defaultBrickSize, level);
        }
        else
        {
            context.Fail("unknown directive '" + std::string(directive) + "' (valid: name, brick_size, brick)");
        }
    }

    context.line = 0; // the remaining checks concern the whole file, not one line
    if (!hasName)
    {
        context.Fail("the level has no name; add a line such as: name My Level");
    }
    if (level.bricks.empty())
    {
        context.Fail("the level has no bricks");
    }
    return level;
}

LevelData LoadLevelFile(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("cannot open level file: " + path.generic_string());
    }

    std::ostringstream contents;
    contents << file.rdbuf();
    return ParseLevel(contents.str(), path.filename().generic_string());
}
} // namespace bb::core
