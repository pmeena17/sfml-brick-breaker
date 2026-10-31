#include "ui/InstructionsScreen.hpp"

#include <array>
#include <string>
#include <string_view>
#include <variant>

#include "core/PowerUp.hpp"
#include "ui/Theme.hpp"

namespace bb::ui
{
namespace
{
using core::Rect;
using core::Vec2;

// C++ Concept: User-defined literals (the `sv` suffix) combined with CTAD
// `"text"sv` is a std::string_view literal (from <string_view>, enabled by the using-directive below, which
// is fine INSIDE a .cpp file or function but not in a header). With every element being a string_view,
// class template argument deduction works out `std::array<std::string_view, 8>` by itself: add or remove a
// line and the element count follows - there is no `[8]` to keep in sync (the original's arrays were
// `int cx[6][6]` and `int xb1[25]`, with the literals 5 and 25 repeated in some 80 loop conditions).
using namespace std::string_view_literals;

constexpr std::array kInstructionLines = {
    "THE AIM OF THE GAME IS TO BREAK ALL THE BRICKS"sv,
    "WITHOUT LETTING THE BALL FALL BELOW THE PADDLE."sv,
    "MOVE THE PADDLE WITH THE LEFT / RIGHT ARROW KEYS"sv,
    "(OR A / D). PRESS SPACE TO LAUNCH, P TO PAUSE."sv,
    "RED BRICKS TAKE TWO HITS."sv,
    "CATCH THE POWER-UPS THAT FALL FROM SOME BRICKS."sv,
    "BEWARE: SOME POWERS MIGHT BE DESTRUCTIVE!"sv,
    "ENJOY THE GAME."sv,
};

constexpr Rect kPanel{.position = {30.0f, 90.0f}, .size = {540.0f, 440.0f}};
constexpr Vec2 kBackCenter{100.0f, 40.0f};
constexpr Vec2 kBackSize{100.0f, 44.0f};
constexpr float kFirstLineY = 125.0f;
constexpr float kLineSpacing = 30.0f;
constexpr float kLegendTitleY = 385.0f;
constexpr float kLegendFirstRowY = 420.0f;
constexpr float kLegendRowSpacing = 36.0f;
constexpr float kSwatchSize = 22.0f;
} // namespace

InstructionsScreen::InstructionsScreen() : m_backButton(Rect::FromCenter(kBackCenter, kBackSize), "BACK", 18.0f)
{
}

std::optional<ScreenCommand> InstructionsScreen::HandleEvent(const InputEvent& event)
{
    if (const auto* key = std::get_if<KeyPressed>(&event))
    {
        if (key->key == Key::Escape || key->key == Key::Enter)
        {
            return ShowMenu{};
        }
        return std::nullopt;
    }

    if (const auto* click = std::get_if<MouseClicked>(&event))
    {
        if (m_backButton.Contains(click->position))
        {
            return ShowMenu{};
        }
    }
    return std::nullopt;
}

std::optional<ScreenCommand> InstructionsScreen::Update(float /*seconds*/)
{
    return std::nullopt;
}

void InstructionsScreen::Render(IRenderer& renderer) const
{
    renderer.Clear(theme::kMenuBackground);
    renderer.FillRect(kPanel, theme::kPanel);
    m_backButton.Render(renderer);

    float y = kFirstLineY;
    for (const std::string_view line : kInstructionLines)
    {
        renderer.DrawLabel(line, {300.0f, y}, 16.0f, theme::kPanelText, TextAnchor::Center);
        y += kLineSpacing;
    }

    renderer.DrawLabel("POWER-UPS", {300.0f, kLegendTitleY}, 18.0f, theme::kPanelText, TextAnchor::Center);

    float rowY = kLegendFirstRowY;
    for (const core::PowerUpDefinition& definition : core::kPowerUpDefinitions)
    {
        renderer.FillRect(Rect::FromCenter({60.0f, rowY}, {kSwatchSize, kSwatchSize}), theme::PowerUpColor(definition.kind));
        const std::string text = std::string(definition.displayName) + " - " + std::string(definition.description);
        renderer.DrawLabel(text, {90.0f, rowY}, 15.0f, theme::kPanelText, TextAnchor::MiddleLeft);
        rowY += kLegendRowSpacing;
    }
}
} // namespace bb::ui
