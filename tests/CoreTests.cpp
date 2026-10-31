#include <cmath>
#include <optional>

#include "TestFramework.hpp"
#include "core/Ball.hpp"
#include "core/Brick.hpp"
#include "core/Collision.hpp"
#include "core/Config.hpp"
#include "core/EffectTracker.hpp"
#include "core/Paddle.hpp"
#include "core/PowerUp.hpp"
#include "core/Rect.hpp"
#include "core/Vec2.hpp"

using namespace bb::core;

TEST_CASE("Vec2: length, normalisation and arithmetic")
{
    const Vec2 a{3.0f, 4.0f};
    CHECK_NEAR(a.Length(), 5.0, 1e-6);
    CHECK_NEAR(a.Normalized().Length(), 1.0, 1e-6);
    CHECK_NEAR(a.Dot(Vec2{2.0f, 1.0f}), 10.0, 1e-6);

    const Vec2 sum = a + Vec2{1.0f, 1.0f};
    const Vec2 expectedSum{4.0f, 5.0f};
    CHECK(sum == expectedSum);

    const Vec2 zero{};
    CHECK(zero.Normalized() == zero);
}

TEST_CASE("Rect: edges, centre, containment and intersection")
{
    const Rect rect = Rect::FromCenter({100.0f, 50.0f}, {40.0f, 20.0f});
    CHECK_NEAR(rect.Left(), 80.0, 1e-6);
    CHECK_NEAR(rect.Right(), 120.0, 1e-6);
    CHECK_NEAR(rect.Top(), 40.0, 1e-6);
    CHECK_NEAR(rect.Bottom(), 60.0, 1e-6);

    const Vec2 center = rect.Center();
    const Vec2 expectedCenter{100.0f, 50.0f};
    CHECK(center == expectedCenter);

    CHECK(rect.Contains({100.0f, 50.0f}));
    CHECK(!rect.Contains({130.0f, 50.0f}));

    const Rect overlapping = Rect::FromCenter({115.0f, 50.0f}, {40.0f, 20.0f});
    const Rect touching = Rect::FromCenter({140.0f, 50.0f}, {40.0f, 20.0f});
    CHECK(rect.Intersects(overlapping));
    CHECK(!rect.Intersects(touching)); // sharing an edge is not an overlap
}

TEST_CASE("Collision: circle hits the top face of a rectangle")
{
    const Rect rect{.position = {100.0f, 100.0f}, .size = {200.0f, 50.0f}};
    const std::optional<CircleHit> hit = TestCircleRect({200.0f, 95.0f}, 10.0f, rect);
    REQUIRE(hit.has_value());
    CHECK_NEAR(hit->normal.x, 0.0, 1e-6);
    CHECK_NEAR(hit->normal.y, -1.0, 1e-6);
    CHECK_NEAR(hit->correction.y, -5.0, 1e-4); // moving up by 5 puts the circle's bottom exactly on the face
}

TEST_CASE("Collision: no overlap returns nullopt")
{
    const Rect rect{.position = {100.0f, 100.0f}, .size = {200.0f, 50.0f}};
    CHECK(!TestCircleRect({200.0f, 80.0f}, 10.0f, rect).has_value());
    CHECK(!TestCircleRect({50.0f, 125.0f}, 10.0f, rect).has_value());
}

TEST_CASE("Collision: side hits use the horizontal normal")
{
    const Rect rect{.position = {100.0f, 100.0f}, .size = {200.0f, 50.0f}};
    const std::optional<CircleHit> left = TestCircleRect({95.0f, 125.0f}, 10.0f, rect);
    const std::optional<CircleHit> right = TestCircleRect({305.0f, 125.0f}, 10.0f, rect);
    REQUIRE(left.has_value());
    REQUIRE(right.has_value());
    CHECK_NEAR(left->normal.x, -1.0, 1e-6);
    CHECK_NEAR(right->normal.x, 1.0, 1e-6);
}

TEST_CASE("Collision: corner hits still produce an axis-aligned normal")
{
    const Rect rect{.position = {100.0f, 100.0f}, .size = {200.0f, 50.0f}};
    const std::optional<CircleHit> hit = TestCircleRect({96.0f, 97.0f}, 10.0f, rect);
    REQUIRE(hit.has_value());
    CHECK(hit->normal.x == 0.0f || hit->normal.y == 0.0f);
    CHECK_NEAR(hit->normal.Length(), 1.0, 1e-6);
}

TEST_CASE("Collision: a centre buried inside the rectangle is pushed out through the nearest face")
{
    const Rect rect{.position = {100.0f, 100.0f}, .size = {200.0f, 50.0f}};
    const std::optional<CircleHit> hit = TestCircleRect({110.0f, 125.0f}, 10.0f, rect);
    REQUIRE(hit.has_value());
    CHECK_NEAR(hit->normal.x, -1.0, 1e-6);
    CHECK_NEAR(110.0f + hit->correction.x, 90.0, 1e-4); // the circle's right edge now touches x = 100
}

TEST_CASE("Ball: reflection mirrors the direction only when moving into the surface")
{
    Ball ball(10.0f, 100.0f);
    ball.Launch({1.0f, -1.0f});
    const float start = ball.Direction().y;

    ball.Reflect({0.0f, 1.0f}); // moving up into a ceiling: flips
    CHECK_NEAR(ball.Direction().y, -start, 1e-6);

    ball.Reflect({0.0f, 1.0f}); // now moving away from the ceiling: ignored
    CHECK_NEAR(ball.Direction().y, -start, 1e-6);
    CHECK_NEAR(ball.Direction().Length(), 1.0, 1e-6);
}

TEST_CASE("Ball: speed scale changes speed but not direction")
{
    Ball ball(10.0f, 100.0f);
    ball.Launch({3.0f, 4.0f});
    CHECK_NEAR(ball.Speed(), 100.0, 1e-4);
    ball.SetSpeedScale(2.0f);
    CHECK_NEAR(ball.Speed(), 200.0, 1e-4);
    CHECK_NEAR(ball.Velocity().x, 120.0, 1e-3);
    CHECK_NEAR(ball.Velocity().y, 160.0, 1e-3);
    ball.SetSpeedScale(1.0f);
    CHECK_NEAR(ball.Speed(), 100.0, 1e-4);
}

TEST_CASE("Paddle: stays inside its corridor, also after changing width")
{
    Paddle paddle({300.0f, 550.0f}, {100.0f, 30.0f}, 500.0f, 10.0f, 590.0f);
    paddle.Move(10.0f, 1);
    CHECK_NEAR(paddle.Bounds().Right(), 590.0, 1e-4);
    paddle.Move(10.0f, -1);
    CHECK_NEAR(paddle.Bounds().Left(), 10.0, 1e-4);

    // Shrinking keeps the centre, so the paddle simply leaves a gap to the wall...
    paddle.SetWidth(50.0f);
    CHECK_NEAR(paddle.Width(), 50.0, 1e-6);
    CHECK_NEAR(paddle.Center().x, 60.0, 1e-4);

    // ...but growing back next to a wall must not push the paddle through it.
    paddle.Move(10.0f, -1);
    CHECK_NEAR(paddle.Bounds().Left(), 10.0, 1e-4);
    paddle.SetWidth(100.0f);
    CHECK_NEAR(paddle.Bounds().Left(), 10.0, 1e-4);
}

TEST_CASE("Brick: hit points count down and never go negative")
{
    Brick brick({{0.0f, 0.0f}, {10.0f, 10.0f}}, 2);
    CHECK(!brick.IsDestroyed());
    brick.TakeHit();
    CHECK_EQ(brick.HitPoints(), 1);
    CHECK(!brick.IsDestroyed());
    brick.TakeHit();
    CHECK(brick.IsDestroyed());
    brick.TakeHit();
    CHECK_EQ(brick.HitPoints(), 0);
    CHECK_EQ(brick.MaxHitPoints(), 2);
}

TEST_CASE("PowerUp: ids round-trip through the definition table")
{
    for (const PowerUpDefinition& definition : kPowerUpDefinitions)
    {
        const std::optional<PowerUpKind> parsed = ParsePowerUpId(definition.id);
        REQUIRE(parsed.has_value());
        CHECK(*parsed == definition.kind);
        CHECK(&GetDefinition(definition.kind) == &definition);
    }
    CHECK(!ParsePowerUpId("bogus").has_value());
    CHECK(!ParsePowerUpId("").has_value());
}

TEST_CASE("EffectTracker: no effects means neutral modifiers")
{
    EffectTracker effects;
    CHECK(!effects.AnyActive());
    CHECK_NEAR(effects.Modifiers().ballSpeedScale, 1.0, 1e-6);
    CHECK_NEAR(effects.Modifiers().paddleWidthScale, 1.0, 1e-6);
}

TEST_CASE("EffectTracker: effects apply, combine and expire")
{
    EffectTracker effects;
    effects.Activate(PowerUpKind::SlowBall);
    CHECK(effects.IsActive(PowerUpKind::SlowBall));
    CHECK_NEAR(effects.Modifiers().ballSpeedScale, 0.5, 1e-6);

    effects.Activate(PowerUpKind::FastBall); // 0.5 * 2.0 cancel out
    CHECK_NEAR(effects.Modifiers().ballSpeedScale, 1.0, 1e-6);

    effects.Activate(PowerUpKind::ShrinkPaddle);
    CHECK_NEAR(effects.Modifiers().paddleWidthScale, 0.5, 1e-6);

    const float duration = GetDefinition(PowerUpKind::SlowBall).durationSeconds;
    const std::vector<PowerUpKind> expired = effects.Update(duration + 0.01f);
    CHECK_EQ(expired.size(), std::size_t{3});
    CHECK(!effects.AnyActive());
    CHECK_NEAR(effects.Modifiers().ballSpeedScale, 1.0, 1e-6);
    CHECK(effects.Update(1.0f).empty()); // nothing expires twice
}

TEST_CASE("EffectTracker: collecting an active effect again refreshes its timer instead of stacking")
{
    EffectTracker effects;
    const float duration = GetDefinition(PowerUpKind::SlowBall).durationSeconds;
    effects.Activate(PowerUpKind::SlowBall);
    (void)effects.Update(duration * 0.6f);
    effects.Activate(PowerUpKind::SlowBall);
    CHECK_NEAR(effects.RemainingSeconds(PowerUpKind::SlowBall), duration, 1e-5);
    CHECK_NEAR(effects.Modifiers().ballSpeedScale, 0.5, 1e-6); // still 0.5, not 0.25
}
