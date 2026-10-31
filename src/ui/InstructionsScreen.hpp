#pragma once

#include <optional>

#include "ui/Button.hpp"
#include "ui/Screen.hpp"

namespace bb::ui
{
// How to play, plus a legend of the power-ups. The legend is generated from core::kPowerUpDefinitions, so a
// new power-up shows up here automatically with its colour, name and description.
class InstructionsScreen final : public Screen
{
public:
    InstructionsScreen();

    [[nodiscard]] std::optional<ScreenCommand> HandleEvent(const InputEvent& event) override;
    [[nodiscard]] std::optional<ScreenCommand> Update(float seconds) override;
    void Render(IRenderer& renderer) const override;

private:
    Button m_backButton;
};
} // namespace bb::ui
