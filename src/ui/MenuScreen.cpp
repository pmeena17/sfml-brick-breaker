#include "ui/MenuScreen.hpp"

#include <algorithm>
#include <string>
#include <variant>

#include "ui/Theme.hpp"

namespace bb::ui
{
namespace
{
using core::Rect;
using core::Vec2;

constexpr Vec2 kTitleCenter{300.0f, 140.0f};
constexpr Vec2 kTitleSize{340.0f, 64.0f};
constexpr Vec2 kInstructionsCenter{110.0f, 40.0f};
constexpr Vec2 kInstructionsSize{180.0f, 44.0f};
constexpr Vec2 kLevelButtonSize{200.0f, 50.0f};
constexpr float kLeftColumnX = 160.0f;
constexpr float kRightColumnX = 440.0f;
constexpr float kCenterColumnX = 300.0f;
constexpr float kFirstRowY = 270.0f;
constexpr float kRowSpacing = 80.0f;
constexpr Vec2 kPagerButtonSize{110.0f, 40.0f};
constexpr float kPagerY = 520.0f;
constexpr Vec2 kPageLabelCenter{300.0f, kPagerY};
constexpr Vec2 kNoticeCenter{300.0f, 585.0f};

// Centre of the button for slot `slot` (0-based) on a page that shows `countOnPage` buttons, laid out in two
// columns. A lone button in the last row is centred, as "LEVEL 5" was in the original.
[[nodiscard]] Vec2 SlotCenter(std::size_t slot, std::size_t countOnPage) noexcept
{
    const std::size_t row = slot / 2;
    const bool isLoneLastButton = (slot + 1 == countOnPage) && (countOnPage % 2 == 1);
    const float x = isLoneLastButton ? kCenterColumnX : ((slot % 2 == 0) ? kLeftColumnX : kRightColumnX);
    return {x, kFirstRowY + static_cast<float>(row) * kRowSpacing};
}
} // namespace

MenuScreen::MenuScreen(const core::LevelRepository& levels, std::size_t focusLevel) : m_levels(levels), m_title(Rect::FromCenter(kTitleCenter, kTitleSize), "BRICK BREAKER", 28.0f), m_instructionsButton(Rect::FromCenter(kInstructionsCenter, kInstructionsSize), "INSTRUCTIONS", 18.0f), m_previousPageButton(Rect::FromCenter({kLeftColumnX, kPagerY}, kPagerButtonSize), "< PREV", 18.0f), m_nextPageButton(Rect::FromCenter({kRightColumnX, kPagerY}, kPagerButtonSize), "NEXT >", 18.0f)
{
    m_page = std::min(focusLevel / kLevelsPerPage, PageCount() - 1);
    RebuildLevelButtons();
}

std::size_t MenuScreen::PageCount() const noexcept
{
    const std::size_t pages = (m_levels.Count() + kLevelsPerPage - 1) / kLevelsPerPage;
    return std::max<std::size_t>(pages, 1);
}

void MenuScreen::RebuildLevelButtons()
{
    m_levelButtons.clear();

    const std::size_t first = m_page * kLevelsPerPage;
    const std::size_t countOnPage = std::min(kLevelsPerPage, m_levels.Count() > first ? m_levels.Count() - first : 0);

    for (std::size_t slot = 0; slot < countOnPage; ++slot)
    {
        const core::LevelData& level = m_levels.At(first + slot);
        m_levelButtons.emplace_back(Rect::FromCenter(SlotCenter(slot, countOnPage), kLevelButtonSize), level.name, 20.0f);
    }
}

void MenuScreen::ChangePage(int delta)
{
    const std::size_t lastPage = PageCount() - 1;
    if (delta < 0 && m_page > 0)
    {
        --m_page;
    }
    else if (delta > 0 && m_page < lastPage)
    {
        ++m_page;
    }
    RebuildLevelButtons();
}

std::optional<ScreenCommand> MenuScreen::HandleEvent(const InputEvent& event)
{
    // C++ Concept: std::get_if - "is this variant currently holding a T?"
    // std::get_if<T>(&variant) returns a pointer to the held value if it is a T and nullptr otherwise, so it
    // composes naturally with `if (const auto* x = ...)`. Use it when you care about ONE alternative and want
    // to ignore the rest; use std::visit (see GameplayScreen) when every alternative needs handling.
    if (const auto* key = std::get_if<KeyPressed>(&event))
    {
        switch (key->key)
        {
        case Key::Escape:
            return QuitApplication{};
        case Key::Left:
            ChangePage(-1);
            break;
        case Key::Right:
            ChangePage(1);
            break;
        default:
            break;
        }
        return std::nullopt;
    }

    const auto* click = std::get_if<MouseClicked>(&event);
    if (click == nullptr)
    {
        return std::nullopt;
    }

    if (m_instructionsButton.Contains(click->position))
    {
        return ShowInstructions{};
    }

    for (std::size_t slot = 0; slot < m_levelButtons.size(); ++slot)
    {
        if (m_levelButtons[slot].Contains(click->position))
        {
            return StartLevel{m_page * kLevelsPerPage + slot};
        }
    }

    if (PageCount() > 1)
    {
        if (m_previousPageButton.Contains(click->position))
        {
            ChangePage(-1);
        }
        else if (m_nextPageButton.Contains(click->position))
        {
            ChangePage(1);
        }
    }
    return std::nullopt;
}

std::optional<ScreenCommand> MenuScreen::Update(float /*seconds*/)
{
    return std::nullopt;
}

void MenuScreen::Render(IRenderer& renderer) const
{
    renderer.Clear(theme::kMenuBackground);

    m_title.Render(renderer);
    m_instructionsButton.Render(renderer);

    for (const Button& button : m_levelButtons)
    {
        button.Render(renderer);
    }

    if (m_levels.Empty())
    {
        renderer.DrawLabel("NO LEVELS FOUND", {kCenterColumnX, kFirstRowY}, 26.0f, theme::kScreenText, TextAnchor::Center);
        renderer.DrawLabel("PUT .level FILES IN assets/levels", {kCenterColumnX, kFirstRowY + 40.0f}, 16.0f, theme::kScreenText, TextAnchor::Center);
    }

    if (PageCount() > 1)
    {
        m_previousPageButton.Render(renderer);
        m_nextPageButton.Render(renderer);
        renderer.DrawLabel("PAGE " + std::to_string(m_page + 1) + " / " + std::to_string(PageCount()), kPageLabelCenter, 18.0f, theme::kScreenText, TextAnchor::Center);
    }

    if (!m_levels.Issues().empty())
    {
        renderer.DrawLabel(std::to_string(m_levels.Issues().size()) + " LEVEL FILE(S) FAILED TO LOAD - SEE CONSOLE", kNoticeCenter, 14.0f, theme::kHighlight, TextAnchor::Center);
    }
}
} // namespace bb::ui
