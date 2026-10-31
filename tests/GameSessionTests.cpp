#include <algorithm>
#include <optional>
#include <utility>
#include <variant>
#include <vector>

#include "TestFramework.hpp"
#include "core/Config.hpp"
#include "core/GameSession.hpp"

using namespace bb::core;

namespace
{
Brick MakeBrick(float centerX, float centerY, float width, float height, int hitPoints = 1, std::optional<PowerUpKind> drop = std::nullopt)
{
    return Brick(Rect::FromCenter({centerX, centerY}, {width, height}), hitPoints, drop);
}

LevelData MakeLevel(std::vector<Brick> bricks)
{
    return LevelData{.name = "Test level", .bricks = std::move(bricks)};
}

// A brick in the top-right corner that practically cannot be destroyed. It keeps a test level from being
// "won" while the test is busy observing something else.
Brick Decoy()
{
    return MakeBrick(560.0f, 40.0f, 40.0f, 20.0f, 1000);
}

// A scripted "player": keeps the paddle under the ball so the ball is never lost.
void TrackBall(GameSession& session)
{
    const float offset = session.GetBall().Position().x - session.GetPaddle().Center().x;
    session.SetPaddleDirection(offset > 4.0f ? 1 : (offset < -4.0f ? -1 : 0));
}

template <typename EventType>
std::size_t CountEvents(const std::vector<GameEvent>& log)
{
    return static_cast<std::size_t>(std::count_if(log.begin(), log.end(), [](const GameEvent& event) { return std::holds_alternative<EventType>(event); }));
}

// Advances the session in fixed steps until `done(session)` is true or `maxSeconds` have elapsed, appending
// every event to `log`. Returns whether `done` became true.
template <typename Predicate>
bool RunUntil(GameSession& session, std::vector<GameEvent>& log, float maxSeconds, bool autoplay, Predicate done)
{
    const int maxSteps = static_cast<int>(maxSeconds / config::kFixedTimeStep);
    for (int step = 0; step < maxSteps; ++step)
    {
        if (autoplay)
        {
            TrackBall(session);
        }
        session.Update(config::kFixedTimeStep);
        for (GameEvent& event : session.DrainEvents())
        {
            log.push_back(event);
        }
        if (done(session))
        {
            return true;
        }
    }
    return false;
}
} // namespace

TEST_CASE("GameSession: starts in Serving with the ball resting on the paddle")
{
    GameSession session(MakeLevel({Decoy()}));
    CHECK(session.Phase() == SessionPhase::Serving);
    CHECK_EQ(session.Score(), 0);
    CHECK_EQ(session.RemainingBricks(), std::size_t{1});
    CHECK_NEAR(session.GetBall().Position().y, session.GetPaddle().Bounds().Top() - session.GetBall().Radius(), 1e-4);
    CHECK_NEAR(session.GetBall().Position().x, session.GetPaddle().Center().x, 1e-4);
}

TEST_CASE("GameSession: while serving, the ball follows the paddle")
{
    GameSession session(MakeLevel({Decoy()}));
    const float startX = session.GetPaddle().Center().x;
    session.SetPaddleDirection(1);
    session.Update(0.1f);
    CHECK(session.GetPaddle().Center().x > startX + 40.0f);
    CHECK_NEAR(session.GetBall().Position().x, session.GetPaddle().Center().x, 1e-4);
    CHECK(session.Phase() == SessionPhase::Serving);
}

TEST_CASE("GameSession: the paddle never leaves the playfield")
{
    GameSession session(MakeLevel({Decoy()}));
    session.SetPaddleDirection(1);
    session.Update(10.0f);
    CHECK(session.GetPaddle().Bounds().Right() <= config::kPlayfield.Right() + 1e-3f);
    session.SetPaddleDirection(-1);
    session.Update(10.0f);
    CHECK(session.GetPaddle().Bounds().Left() >= config::kPlayfield.Left() - 1e-3f);
}

TEST_CASE("GameSession: launching starts play exactly once")
{
    GameSession session(MakeLevel({Decoy()}));
    session.LaunchBall();
    CHECK(session.Phase() == SessionPhase::Playing);
    session.LaunchBall(); // second launch is ignored
    const std::vector<GameEvent> events = session.DrainEvents();
    CHECK_EQ(CountEvents<BallLaunched>(events), std::size_t{1});
    CHECK(session.DrainEvents().empty()); // draining clears the queue
}

TEST_CASE("GameSession: the ball bounces off all walls and the paddle and never escapes")
{
    GameSession session(MakeLevel({Decoy()}));
    session.LaunchBall();

    const float radius = session.GetBall().Radius();
    const Rect& field = config::kPlayfield;
    bool leftField = false;
    bool sawRightWallBounce = false;
    bool sawLeftWallBounce = false;
    bool sawCeilingBounce = false;
    bool sawPaddleBounce = false;
    Vec2 previousDirection = session.GetBall().Direction();

    // 40 simulated seconds of perfect play.
    std::vector<GameEvent> log;
    const auto observe = [&](const GameSession& s)
    {
        const Vec2 position = s.GetBall().Position();
        const Vec2 direction = s.GetBall().Direction();
        leftField = leftField || position.x - radius < field.Left() - 0.01f || position.x + radius > field.Right() + 0.01f || position.y - radius < field.Top() - 0.01f;
        sawRightWallBounce = sawRightWallBounce || (previousDirection.x > 0.0f && direction.x < 0.0f);
        sawLeftWallBounce = sawLeftWallBounce || (previousDirection.x < 0.0f && direction.x > 0.0f);
        sawCeilingBounce = sawCeilingBounce || (previousDirection.y < 0.0f && direction.y > 0.0f);
        sawPaddleBounce = sawPaddleBounce || (previousDirection.y > 0.0f && direction.y < 0.0f);
        previousDirection = direction;
        return false;
    };
    RunUntil(session, log, 40.0f, true, observe);

    CHECK(session.Phase() == SessionPhase::Playing);
    CHECK(!leftField);
    CHECK(sawRightWallBounce);
    CHECK(sawLeftWallBounce);
    CHECK(sawCeilingBounce);
    CHECK(sawPaddleBounce);
    CHECK_NEAR(session.GetBall().Direction().Length(), 1.0, 1e-3);
    CHECK_NEAR(session.GetBall().Speed(), config::ball::kBaseSpeed, 1e-3); // bouncing never changes the speed
}

TEST_CASE("GameSession: missing the ball loses the level")
{
    GameSession session(MakeLevel({Decoy()}));
    session.SetPaddleDirection(-1); // run into the left corner and stay there
    session.LaunchBall();

    std::vector<GameEvent> log;
    const bool lost = RunUntil(session, log, 60.0f, false, [](const GameSession& s) { return s.Phase() != SessionPhase::Playing; });
    CHECK(lost);
    CHECK(session.Phase() == SessionPhase::Lost);
    CHECK_EQ(CountEvents<BallLost>(log), std::size_t{1});

    // A finished session is frozen.
    const Vec2 frozen = session.GetBall().Position();
    session.Update(1.0f);
    CHECK(session.GetBall().Position() == frozen);
}

TEST_CASE("GameSession: destroying the last brick wins the level and scores it")
{
    // The brick sits on the ball's opening diagonal, so the first bounce hits it.
    GameSession session(MakeLevel({MakeBrick(350.0f, 450.0f, 120.0f, 20.0f)}));
    session.LaunchBall();

    std::vector<GameEvent> log;
    const bool won = RunUntil(session, log, 20.0f, true, [](const GameSession& s) { return s.Phase() != SessionPhase::Playing; });
    CHECK(won);
    CHECK(session.Phase() == SessionPhase::Won);
    CHECK_EQ(session.Score(), config::kScorePerBrick);
    CHECK_EQ(session.RemainingBricks(), std::size_t{0});
    CHECK_EQ(CountEvents<BrickHit>(log), std::size_t{1});
    CHECK_EQ(CountEvents<LevelCleared>(log), std::size_t{1});
}

TEST_CASE("GameSession: a two-hit brick survives the first hit and falls on the second")
{
    // A brick spanning the whole width: the ball is certain to hit it from below, again and again.
    GameSession session(MakeLevel({MakeBrick(300.0f, 150.0f, 580.0f, 20.0f, 2)}));
    session.LaunchBall();

    std::vector<GameEvent> log;
    const bool won = RunUntil(session, log, 60.0f, true, [](const GameSession& s) { return s.Phase() != SessionPhase::Playing; });
    CHECK(won);
    CHECK(session.Phase() == SessionPhase::Won);
    CHECK_EQ(session.Score(), config::kScorePerBrick); // scored once, for the destroyed brick, not per hit

    std::vector<BrickHit> hits;
    for (const GameEvent& event : log)
    {
        if (const auto* hit = std::get_if<BrickHit>(&event))
        {
            hits.push_back(*hit);
        }
    }
    REQUIRE(hits.size() == std::size_t{2});
    CHECK(!hits[0].destroyed);
    CHECK(hits[1].destroyed);
}

// Geometry used by the power-up tests: a 60x20 brick sits just above the paddle's starting position. The opening
// bounce destroys it within a few frames and the capsule has only ~20 px to fall, so a player who merely keeps
// the paddle under the ball also catches the capsule (a brick further away would make "follow the ball" and
// "follow the capsule" contradict each other).
Brick PowerUpBrick(PowerUpKind kind)
{
    return MakeBrick(340.0f, 500.0f, 60.0f, 20.0f, 1, kind);
}

TEST_CASE("GameSession: a destroyed brick drops a capsule; catching it shrinks the paddle until it expires")
{
    GameSession session(MakeLevel({Decoy(), PowerUpBrick(PowerUpKind::ShrinkPaddle)}));
    session.LaunchBall();
    const float normalWidth = session.GetPaddle().Width();

    std::vector<GameEvent> log;
    const bool collected = RunUntil(session, log, 15.0f, true, [&log](const GameSession&) { return CountEvents<PowerUpCollected>(log) > 0; });
    REQUIRE(collected);
    CHECK_EQ(CountEvents<PowerUpSpawned>(log), std::size_t{1});
    CHECK(session.Effects().IsActive(PowerUpKind::ShrinkPaddle));
    CHECK_NEAR(session.GetPaddle().Width(), normalWidth * 0.5f, 1e-3);
    CHECK(session.FallingPowerUps().empty());

    const bool expired = RunUntil(session, log, 15.0f, true, [&log](const GameSession&) { return CountEvents<PowerUpExpired>(log) > 0; });
    REQUIRE(expired);
    CHECK(!session.Effects().IsActive(PowerUpKind::ShrinkPaddle));
    CHECK_NEAR(session.GetPaddle().Width(), normalWidth, 1e-3);
}

TEST_CASE("GameSession: the fast-ball power-up doubles the speed and expiry restores it exactly")
{
    GameSession session(MakeLevel({Decoy(), PowerUpBrick(PowerUpKind::FastBall)}));
    session.LaunchBall();

    std::vector<GameEvent> log;
    REQUIRE(RunUntil(session, log, 15.0f, true, [&log](const GameSession&) { return CountEvents<PowerUpCollected>(log) > 0; }));
    CHECK_NEAR(session.GetBall().Speed(), config::ball::kBaseSpeed * 2.0f, 1e-3);

    REQUIRE(RunUntil(session, log, 15.0f, true, [&log](const GameSession&) { return CountEvents<PowerUpExpired>(log) > 0; }));
    CHECK_NEAR(session.GetBall().Speed(), config::ball::kBaseSpeed, 1e-3); // no drift, unlike repeated vx /= 2
    CHECK_NEAR(session.GetBall().Direction().Length(), 1.0, 1e-3);
}
