#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "core/LevelRepository.hpp"
#include "ui/Button.hpp"
#include "ui/Screen.hpp"

namespace bb::ui
{
// The start screen: title, an INSTRUCTIONS button and one button per level.
//
// The level buttons are generated from the LevelRepository, so adding a level file adds a button; when
// there are more levels than fit on one page, PREV/NEXT buttons appear. The original had five hand-placed
// rectangles and five hand-placed texts.
class MenuScreen final : public Screen
{
public:
    // C++ Concept: `static constexpr` class members
    // A compile-time constant that belongs to the class itself (one copy, not one per object), declared
    // right where it is meaningful: MenuScreen::kLevelsPerPage. Since C++17 static constexpr data members are
    // implicitly inline, so no separate definition in a .cpp file is needed.
    static constexpr std::size_t kLevelsPerPage = 6;

    // `levels` must outlive the screen. The menu opens on the page containing `focusLevel`.
    MenuScreen(const core::LevelRepository& levels, std::size_t focusLevel);

    [[nodiscard]] std::optional<ScreenCommand> HandleEvent(const InputEvent& event) override;
    [[nodiscard]] std::optional<ScreenCommand> Update(float seconds) override;
    void Render(IRenderer& renderer) const override;

    [[nodiscard]] std::size_t Page() const noexcept
    {
        return m_page;
    }

    [[nodiscard]] std::size_t PageCount() const noexcept;

    // The buttons of the current page, in level order (used by the tests to simulate clicks).
    [[nodiscard]] const std::vector<Button>& LevelButtons() const noexcept
    {
        return m_levelButtons;
    }

private:
    void RebuildLevelButtons();
    void ChangePage(int delta);

    const core::LevelRepository& m_levels;
    std::size_t m_page{0};
    std::vector<Button> m_levelButtons;
    Button m_title;
    Button m_instructionsButton;
    Button m_previousPageButton;
    Button m_nextPageButton;
};
} // namespace bb::ui
