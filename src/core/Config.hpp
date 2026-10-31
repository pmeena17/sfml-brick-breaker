#pragma once

#include "core/Rect.hpp"
#include "core/Vec2.hpp"

// Every tunable number in the game lives in this one file.
//
// The original program scattered literals such as 530, 550, 590, 591, 592, 525, 610 and 1000 through
// the code, so changing the paddle height meant hunting for every place that "knew" it. Here each
// value has ONE name, one unit, and one place to change it.
//
// Units: lengths are logical pixels, speeds are pixels per second, durations are seconds.
//
// C++ Concept: `inline constexpr` variables (C++17) instead of #define or `static const`
//   * `#define PADDLE_Y 550` is text substitution: no type, no scope, ignored by the debugger.
//   * `constexpr` gives a typed constant that is computed at compile time and usable in
//     static_assert and as template arguments.
//   * `inline` lets the SAME variable be defined in a header included from many .cpp files without
//     violating the One Definition Rule - there is exactly one object in the final program.
// Grouping them in (nested) namespaces gives tidy names: config::paddle::kWidth.
namespace bb::core::config
{
// ---------------------------------------------------------------------------------------------
// Window / playfield
// ---------------------------------------------------------------------------------------------
inline constexpr float kPlayfieldMargin = 10.0f;
inline constexpr float kHudHeight = 40.0f;

// Total logical window size: the 600x600 arena of the original game plus a strip for the HUD.
inline constexpr Vec2 kWindowSize{600.0f, 640.0f};

// The rectangle the ball bounces inside (x: 10..590, y: 10..590 - same as the original black arena).
inline constexpr Rect kPlayfield{.position = {kPlayfieldMargin, kPlayfieldMargin}, .size = {kWindowSize.x - 2.0f * kPlayfieldMargin, kWindowSize.y - kHudHeight - 2.0f * kPlayfieldMargin}};

// ---------------------------------------------------------------------------------------------
// Simulation
// ---------------------------------------------------------------------------------------------
// The game logic advances in fixed steps (see Application::Run for why). 120 Hz keeps the ball's
// per-step movement far smaller than a brick, so it can never tunnel through one.
inline constexpr float kFixedTimeStep = 1.0f / 120.0f;
inline constexpr float kMaxFrameTime = 0.25f;
inline constexpr int kScorePerBrick = 1;

namespace ball
{
inline constexpr float kRadius = 10.0f;
inline constexpr float kBaseSpeed = 320.0f;
inline constexpr Vec2 kLaunchDirection{1.0f, -1.0f}; // up and to the right, like the original (vx=2, vy=-2); normalised on launch
} // namespace ball

namespace paddle
{
inline constexpr float kCenterY = 550.0f;
inline constexpr float kWidth = 100.0f;
inline constexpr float kHeight = 30.0f;
inline constexpr float kSpeed = 520.0f;
} // namespace paddle

namespace power_up
{
inline constexpr Vec2 kSize{30.0f, 30.0f};
inline constexpr float kFallSpeed = 110.0f;
} // namespace power_up

namespace brick
{
inline constexpr Vec2 kDefaultSize{60.0f, 25.0f}; // used when a level file gives neither brick_size nor w=/h=
inline constexpr int kMaxHitPoints = 9;
} // namespace brick

// C++ Concept: static_assert - compile-time sanity checks
// A static_assert is evaluated by the compiler; if the condition is false the build FAILS with your
// message. It is the cheapest possible test: it costs nothing at run time and runs on every build.
// Use it to encode assumptions between constants so nobody can silently break them by editing one number.
static_assert(paddle::kCenterY + paddle::kHeight * 0.5f < kPlayfield.Bottom(), "the paddle must fit inside the playfield");
static_assert(kPlayfield.Bottom() + kPlayfieldMargin <= kWindowSize.y - kHudHeight, "the HUD strip must not overlap the playfield");
static_assert(ball::kBaseSpeed * kFixedTimeStep * 2.0f < ball::kRadius, "even at double speed the ball must move less than its radius per step (no tunnelling)");
} // namespace bb::core::config
