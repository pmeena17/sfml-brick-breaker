#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "TestFramework.hpp"
#include "core/Config.hpp"
#include "core/LevelRepository.hpp"
#include "ui/GameplayScreen.hpp"
#include "ui/InstructionsScreen.hpp"
#include "ui/MenuScreen.hpp"
#include "ui/ResultScreen.hpp"
#include "ui/ScreenNavigator.hpp"

using namespace bb::ui;
using bb::core::Brick;
using bb::core::LevelData;
using bb::core::LevelRepository;
using bb::core::Rect;
using bb::core::Vec2;

namespace
{
// A renderer that draws nothing and remembers what it was asked to draw. This is the payoff of coding the
// screens against the IRenderer interface: they can be exercised without a window or a GPU.
class RecordingRenderer final : public IRenderer
{
public:
    int clears{0};
    int rects{0};
    int circles{0};
    std::vector<std::string> labels;

    void Clear(Color /*color*/) override
    {
        ++clears;
    }

    void FillRect(const bb::core::Rect& /*rect*/, Color /*color*/) override
    {
        ++rects;
    }

    void FillCircle(bb::core::Vec2 /*center*/, float /*radius*/, Color /*color*/) override
    {
        ++circles;
    }

    void DrawLabel(std::string_view text, bb::core::Vec2 /*anchor*/, float /*fontSize*/, Color /*color*/, TextAnchor /*alignment*/) override
    {
        labels.emplace_back(text);
    }

    [[nodiscard]] bool HasLabel(const std::string& text) const
    {
        for (const std::string& label : labels)
        {
            if (label.find(text) != std::string::npos)
            {
                return true;
            }
        }
        return false;
    }
};

LevelData MakeLevel(const std::string& name)
{
    return LevelData{.name = name, .bricks = {Brick(Rect::FromCenter({560.0f, 40.0f}, {40.0f, 20.0f}), 1000)}};
}

LevelRepository MakeRepository(std::size_t count)
{
    std::vector<LevelData> levels;
    for (std::size_t index = 0; index < count; ++index)
    {
        levels.push_back(MakeLevel("Level " + std::to_string(index + 1)));
    }
    return LevelRepository::FromLevels(std::move(levels));
}

InputEvent Click(Vec2 position)
{
    return MouseClicked{position};
}

InputEvent Press(Key key)
{
    return KeyPressed{key};
}

template <typename Command>
const Command* As(const std::optional<ScreenCommand>& command)
{
    return command.has_value() ? std::get_if<Command>(&*command) : nullptr;
}
} // namespace

TEST_CASE("MenuScreen: one button per level, each starting its own level")
{
    const LevelRepository levels = MakeRepository(5);
    MenuScreen menu(levels, 0);
    REQUIRE(menu.LevelButtons().size() == std::size_t{5});
    CHECK_EQ(menu.PageCount(), std::size_t{1});

    for (std::size_t index = 0; index < 5; ++index)
    {
        const std::optional<ScreenCommand> command = menu.HandleEvent(Click(menu.LevelButtons()[index].Bounds().Center()));
        const StartLevel* start = As<StartLevel>(command);
        REQUIRE(start != nullptr);
        CHECK_EQ(start->levelIndex, index);
    }
    CHECK_EQ(menu.LevelButtons()[2].Label(), std::string("Level 3"));
}

TEST_CASE("MenuScreen: a lone last button is centred, the others use two columns")
{
    const LevelRepository levels = MakeRepository(5);
    const MenuScreen menu(levels, 0);
    const auto& buttons = menu.LevelButtons();
    CHECK(buttons[0].Bounds().Center().x < buttons[1].Bounds().Center().x);
    CHECK_NEAR(buttons[0].Bounds().Center().y, buttons[1].Bounds().Center().y, 1e-4);
    CHECK_NEAR(buttons[4].Bounds().Center().x, bb::core::config::kWindowSize.x * 0.5f, 1e-4);
}

TEST_CASE("MenuScreen: other clicks and keys")
{
    const LevelRepository levels = MakeRepository(3);
    MenuScreen menu(levels, 0);

    CHECK(!menu.HandleEvent(Click({1.0f, 639.0f})).has_value());
    CHECK(As<QuitApplication>(menu.HandleEvent(Press(Key::Escape))) != nullptr);
    CHECK(As<ShowInstructions>(menu.HandleEvent(Click({110.0f, 40.0f}))) != nullptr);
    CHECK(!menu.HandleEvent(Press(Key::Space)).has_value());
}

TEST_CASE("MenuScreen: more levels than fit on a page get paging")
{
    const LevelRepository levels = MakeRepository(8);
    MenuScreen menu(levels, 0);
    CHECK_EQ(menu.PageCount(), std::size_t{2});
    CHECK_EQ(menu.LevelButtons().size(), MenuScreen::kLevelsPerPage);

    (void)menu.HandleEvent(Press(Key::Right));
    CHECK_EQ(menu.Page(), std::size_t{1});
    REQUIRE(menu.LevelButtons().size() == std::size_t{2});
    const StartLevel* start = nullptr;
    const std::optional<ScreenCommand> command = menu.HandleEvent(Click(menu.LevelButtons()[0].Bounds().Center()));
    start = As<StartLevel>(command);
    REQUIRE(start != nullptr);
    CHECK_EQ(start->levelIndex, MenuScreen::kLevelsPerPage); // first level of page two

    (void)menu.HandleEvent(Press(Key::Right)); // already on the last page
    CHECK_EQ(menu.Page(), std::size_t{1});
    (void)menu.HandleEvent(Press(Key::Left));
    CHECK_EQ(menu.Page(), std::size_t{0});

    const MenuScreen focused(levels, 7); // opens on the page that contains level 8
    CHECK_EQ(focused.Page(), std::size_t{1});
}

TEST_CASE("MenuScreen: renders sensibly with no levels at all")
{
    const LevelRepository levels = MakeRepository(0);
    MenuScreen menu(levels, 0);
    CHECK(menu.LevelButtons().empty());
    RecordingRenderer renderer;
    menu.Render(renderer);
    CHECK(renderer.HasLabel("NO LEVELS FOUND"));
}

TEST_CASE("ResultScreen: buttons depend on the outcome and carry their own commands")
{
    ResultScreen won(ResultKind::Won, 25, 1, true);
    REQUIRE(won.ActionCount() == std::size_t{3});
    const std::optional<ScreenCommand> next = won.HandleEvent(Click(won.ActionButton(0).Bounds().Center()));
    REQUIRE(As<StartLevel>(next) != nullptr);
    CHECK_EQ(As<StartLevel>(next)->levelIndex, std::size_t{2});
    const std::optional<ScreenCommand> menu = won.HandleEvent(Click(won.ActionButton(2).Bounds().Center()));
    CHECK(As<ShowMenu>(menu) != nullptr);

    ResultScreen wonLast(ResultKind::Won, 25, 4, false);
    CHECK_EQ(wonLast.ActionCount(), std::size_t{2}); // no NEXT LEVEL after the last level

    ResultScreen lost(ResultKind::Lost, 7, 3, true);
    CHECK_EQ(lost.ActionCount(), std::size_t{2});
    const std::optional<ScreenCommand> retry = lost.HandleEvent(Press(Key::Enter));
    REQUIRE(As<StartLevel>(retry) != nullptr);
    CHECK_EQ(As<StartLevel>(retry)->levelIndex, std::size_t{3});
    CHECK(As<ShowMenu>(lost.HandleEvent(Press(Key::Escape))) != nullptr);

    RecordingRenderer renderer;
    lost.Render(renderer);
    CHECK(renderer.HasLabel("GAME OVER"));
    CHECK(renderer.HasLabel("YOUR SCORE IS : 7"));
}

TEST_CASE("InstructionsScreen: back returns to the menu and the legend lists every power-up")
{
    InstructionsScreen screen;
    CHECK(As<ShowMenu>(screen.HandleEvent(Click({100.0f, 40.0f}))) != nullptr);
    CHECK(As<ShowMenu>(screen.HandleEvent(Press(Key::Escape))) != nullptr);
    CHECK(!screen.HandleEvent(Click({500.0f, 600.0f})).has_value());

    RecordingRenderer renderer;
    screen.Render(renderer);
    for (const bb::core::PowerUpDefinition& definition : bb::core::kPowerUpDefinitions)
    {
        CHECK(renderer.HasLabel(std::string(definition.displayName)));
    }
}

TEST_CASE("GameplayScreen: keys drive the session, escape leaves, focus loss pauses")
{
    const LevelRepository levels = MakeRepository(2);
    GameplayScreen screen(levels, 1);
    CHECK(screen.Session().Phase() == bb::core::SessionPhase::Serving);

    const float startX = screen.Session().GetPaddle().Center().x;
    (void)screen.HandleEvent(Press(Key::Right));
    (void)screen.Update(0.1f);
    CHECK(screen.Session().GetPaddle().Center().x > startX);
    (void)screen.HandleEvent(KeyReleased{Key::Right});
    const float stoppedX = screen.Session().GetPaddle().Center().x;
    (void)screen.Update(0.1f);
    CHECK_NEAR(screen.Session().GetPaddle().Center().x, stoppedX, 1e-4);

    (void)screen.HandleEvent(Press(Key::Space));
    CHECK(screen.Session().Phase() == bb::core::SessionPhase::Playing);

    (void)screen.HandleEvent(Press(Key::P));
    CHECK(screen.IsPaused());
    const Vec2 pausedBall = screen.Session().GetBall().Position();
    (void)screen.Update(0.5f);
    CHECK(screen.Session().GetBall().Position() == pausedBall); // paused: nothing moves
    (void)screen.HandleEvent(Press(Key::P));
    CHECK(!screen.IsPaused());

    (void)screen.HandleEvent(FocusLost{});
    CHECK(screen.IsPaused());

    const std::optional<ScreenCommand> leave = screen.HandleEvent(Press(Key::Escape));
    REQUIRE(As<ShowMenu>(leave) != nullptr);
    CHECK_EQ(As<ShowMenu>(leave)->focusLevel, std::size_t{1});
}

TEST_CASE("GameplayScreen: renders bricks, paddle, ball and HUD")
{
    const LevelRepository levels = MakeRepository(1);
    GameplayScreen screen(levels, 0);
    RecordingRenderer renderer;
    screen.Render(renderer);
    CHECK_EQ(renderer.clears, 1);
    CHECK_EQ(renderer.circles, 1);
    CHECK(renderer.rects >= 3); // playfield + one brick + paddle
    CHECK(renderer.HasLabel("SCORE 0"));
    CHECK(renderer.HasLabel("PRESS SPACE TO LAUNCH"));
}

TEST_CASE("ScreenNavigator: menu -> instructions -> menu -> level -> result -> menu -> quit")
{
    const LevelRepository levels = MakeRepository(2);
    ScreenNavigator navigator(levels);
    CHECK(dynamic_cast<const MenuScreen*>(&navigator.Current()) != nullptr);

    navigator.HandleEvent(Click({110.0f, 40.0f}));
    CHECK(dynamic_cast<const InstructionsScreen*>(&navigator.Current()) != nullptr);
    navigator.HandleEvent(Press(Key::Escape));
    REQUIRE(dynamic_cast<const MenuScreen*>(&navigator.Current()) != nullptr);

    const auto* menu = dynamic_cast<const MenuScreen*>(&navigator.Current());
    navigator.HandleEvent(Click(menu->LevelButtons()[0].Bounds().Center()));
    CHECK(dynamic_cast<const GameplayScreen*>(&navigator.Current()) != nullptr);

    // Hold left (the paddle runs into the corner), launch, and wait for the ball to be lost.
    navigator.HandleEvent(Press(Key::Left));
    navigator.HandleEvent(Press(Key::Space));
    for (int step = 0; step < 120 * 60 && dynamic_cast<const ResultScreen*>(&navigator.Current()) == nullptr; ++step)
    {
        navigator.Update(bb::core::config::kFixedTimeStep);
    }
    const auto* result = dynamic_cast<const ResultScreen*>(&navigator.Current());
    REQUIRE(result != nullptr);
    CHECK_EQ(result->ActionCount(), std::size_t{2});

    navigator.HandleEvent(Press(Key::Escape)); // back to the menu
    CHECK(dynamic_cast<const MenuScreen*>(&navigator.Current()) != nullptr);

    CHECK(!navigator.ShouldQuit());
    navigator.HandleEvent(Press(Key::Escape));
    CHECK(navigator.ShouldQuit());
}
