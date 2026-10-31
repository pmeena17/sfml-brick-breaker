#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "TestFramework.hpp"
#include "core/LevelParser.hpp"
#include "core/LevelRepository.hpp"

using namespace bb::core;

namespace
{
// Parses `text` and returns the 1-based line of the error it must throw (or -1 if nothing was thrown).
int ErrorLine(std::string_view text)
{
    try
    {
        (void)ParseLevel(text, "test.level");
    }
    catch (const LevelParseError& error)
    {
        return error.Line();
    }
    return -1;
}

// C++ Concept: RAII for test fixtures
// The destructor removes the temporary directory no matter how the test ends (normally, a failed REQUIRE,
// an exception). Resource cleanup tied to object lifetime is the heart of RAII; the same idea powers
// std::unique_ptr, std::ifstream and std::lock_guard.
class TempDirectory
{
public:
    explicit TempDirectory(const std::string& name) : m_path(std::filesystem::temp_directory_path() / name)
    {
        std::filesystem::remove_all(m_path);
        std::filesystem::create_directories(m_path);
    }

    ~TempDirectory()
    {
        std::error_code ignored;
        std::filesystem::remove_all(m_path, ignored);
    }

    TempDirectory(const TempDirectory&) = delete;
    TempDirectory& operator=(const TempDirectory&) = delete;

    [[nodiscard]] const std::filesystem::path& Path() const noexcept
    {
        return m_path;
    }

    void Write(const std::string& fileName, std::string_view contents) const
    {
        std::ofstream file(m_path / fileName, std::ios::binary);
        file << contents;
    }

private:
    std::filesystem::path m_path;
};

constexpr std::string_view kGoodLevel = "name Good\nbrick 100 100\n";
} // namespace

TEST_CASE("LevelParser: parses names, default size, per-brick options and power-ups")
{
    const LevelData level = ParseLevel("# a comment\n\nname My Level  # trailing comment\nbrick_size 40 20\nbrick 100 100\nbrick 200 100 hp=2 power=fast_ball w=60\n", "test.level");
    CHECK_EQ(level.name, std::string("My Level"));
    REQUIRE(level.bricks.size() == std::size_t{2});

    const Brick& plain = level.bricks[0];
    CHECK_NEAR(plain.Bounds().Left(), 80.0, 1e-4);
    CHECK_NEAR(plain.Bounds().Top(), 90.0, 1e-4);
    CHECK_NEAR(plain.Bounds().size.x, 40.0, 1e-4);
    CHECK_NEAR(plain.Bounds().size.y, 20.0, 1e-4);
    CHECK_EQ(plain.HitPoints(), 1);
    CHECK(!plain.Drop().has_value());

    const Brick& special = level.bricks[1];
    CHECK_NEAR(special.Bounds().size.x, 60.0, 1e-4);
    CHECK_NEAR(special.Bounds().size.y, 20.0, 1e-4);
    CHECK_EQ(special.HitPoints(), 2);
    REQUIRE(special.Drop().has_value());
    CHECK(*special.Drop() == PowerUpKind::FastBall);
}

TEST_CASE("LevelParser: tolerates Windows line endings and a UTF-8 byte-order mark")
{
    const LevelData level = ParseLevel("\xEF\xBB\xBFname Windows\r\nbrick 100 100\r\nbrick 200 100\r\n", "test.level");
    CHECK_EQ(level.name, std::string("Windows"));
    CHECK_EQ(level.bricks.size(), std::size_t{2});
}

TEST_CASE("LevelParser: uses the default brick size when none is given")
{
    const LevelData level = ParseLevel("name Defaults\nbrick 100 100\n", "test.level");
    REQUIRE(level.bricks.size() == std::size_t{1});
    CHECK_NEAR(level.bricks[0].Bounds().size.x, 60.0, 1e-4);
    CHECK_NEAR(level.bricks[0].Bounds().size.y, 25.0, 1e-4);
}

TEST_CASE("LevelParser: errors carry the offending line number")
{
    CHECK_EQ(ErrorLine("name X\nbrick 100 100\nfrobnicate 1 2\n"), 3);
    CHECK_EQ(ErrorLine("name X\nbrick abc 100\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100 100 power=laser\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100 100 hp=0\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100 100 hp=99\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100 100 color=red\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick 100 100 hp\n"), 2);
    CHECK_EQ(ErrorLine("name X\nname Y\nbrick 100 100\n"), 2);
    CHECK_EQ(ErrorLine("name X\nbrick_size 40\nbrick 100 100\n"), 2);
}

TEST_CASE("LevelParser: bricks must lie inside the playfield and above the paddle")
{
    CHECK_EQ(ErrorLine("name X\nbrick 5 100\n"), 2);     // sticks out to the left
    CHECK_EQ(ErrorLine("name X\nbrick 590 100\n"), 2);   // sticks out to the right
    CHECK_EQ(ErrorLine("name X\nbrick 100 5\n"), 2);     // sticks out of the top
    CHECK_EQ(ErrorLine("name X\nbrick 100 540\n"), 2);   // down on the paddle
    CHECK_EQ(ErrorLine(kGoodLevel), -1);
}

TEST_CASE("LevelParser: whole-file problems report line 0")
{
    CHECK_EQ(ErrorLine("brick 100 100\n"), 0);       // no name
    CHECK_EQ(ErrorLine("name Empty\n"), 0);           // no bricks
    CHECK_EQ(ErrorLine(""), 0);
}

TEST_CASE("LevelParser: error messages name the file and the line")
{
    try
    {
        (void)ParseLevel("name X\nbrick 1 2 3 4\n", "level_09.level");
        CHECK(false);
    }
    catch (const LevelParseError& error)
    {
        const std::string message = error.what();
        CHECK(message.rfind("level_09.level:2: ", 0) == 0);
    }
}

TEST_CASE("LevelRepository: loads valid files in name order and reports broken ones")
{
    TempDirectory directory("bb_tests_repository_mixed");
    directory.Write("b_second.level", "name Second\nbrick 100 100\n");
    directory.Write("a_first.level", "name First\nbrick 100 100\n");
    directory.Write("broken.level", "name Broken\nbrick nonsense\n");
    directory.Write("notes.txt", "not a level");

    const LevelRepository repository = LevelRepository::LoadFromDirectory(directory.Path());
    REQUIRE(repository.Count() == std::size_t{2});
    CHECK_EQ(repository.At(0).name, std::string("First"));
    CHECK_EQ(repository.At(1).name, std::string("Second"));
    CHECK(repository.HasNext(0));
    CHECK(!repository.HasNext(1));

    REQUIRE(repository.Issues().size() == std::size_t{1});
    CHECK_EQ(repository.Issues()[0].file.filename().string(), std::string("broken.level"));
    CHECK_THROWS_AS(repository.At(2), std::out_of_range);
}

TEST_CASE("LevelRepository: a missing directory yields an empty repository, not a crash")
{
    const LevelRepository repository = LevelRepository::LoadFromDirectory("this/directory/does/not/exist");
    CHECK(repository.Empty());
    CHECK_EQ(repository.Issues().size(), std::size_t{1});
}

#ifdef BB_ASSETS_DIR
TEST_CASE("Shipped levels: all five original levels load and keep the original power-up and durable-brick layout")
{
    const LevelRepository repository = LevelRepository::LoadFromDirectory(std::filesystem::path(BB_ASSETS_DIR) / "levels");
    for (const LevelLoadIssue& issue : repository.Issues())
    {
        std::cerr << issue.file << ": " << issue.message << "\n";
    }
    CHECK(repository.Issues().empty());
    REQUIRE(repository.Count() >= std::size_t{5});

    for (std::size_t levelIndex = 0; levelIndex < 5; ++levelIndex)
    {
        const LevelData& level = repository.At(levelIndex);
        REQUIRE(level.bricks.size() == std::size_t{25});

        // Original rules, by brick index: 10..14 took two hits; bricks 6, 15 and 16 carried the three powers.
        for (std::size_t brickIndex = 0; brickIndex < level.bricks.size(); ++brickIndex)
        {
            const Brick& brick = level.bricks[brickIndex];
            CHECK_EQ(brick.HitPoints(), (brickIndex >= 10 && brickIndex < 15) ? 2 : 1);
        }
        REQUIRE(level.bricks[6].Drop().has_value());
        REQUIRE(level.bricks[15].Drop().has_value());
        REQUIRE(level.bricks[16].Drop().has_value());
        CHECK(*level.bricks[6].Drop() == PowerUpKind::SlowBall);
        CHECK(*level.bricks[15].Drop() == PowerUpKind::FastBall);
        CHECK(*level.bricks[16].Drop() == PowerUpKind::ShrinkPaddle);
    }

    // Spot-check the coordinates recovered from the original program's layout code.
    const LevelData& level1 = repository.At(0);
    CHECK_NEAR(level1.bricks[0].Bounds().Center().x, 80.0, 1e-4);
    CHECK_NEAR(level1.bricks[0].Bounds().Center().y, 40.0, 1e-4);
    CHECK_NEAR(level1.bricks[24].Bounds().Center().x, 504.0, 1e-4);
    CHECK_NEAR(level1.bricks[24].Bounds().Center().y, 264.0, 1e-4);
    CHECK_NEAR(level1.bricks[0].Bounds().size.x, 100.0, 1e-4);
}
#endif
