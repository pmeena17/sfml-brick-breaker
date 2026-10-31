# Pure game logic. No graphics, no windowing, no I/O except reading level files.
add_library(bb_core STATIC
    Ball.hpp
    Brick.hpp
    Collision.cpp
    Collision.hpp
    Config.hpp
    EffectTracker.cpp
    EffectTracker.hpp
    GameEvents.hpp
    GameSession.cpp
    GameSession.hpp
    LevelData.hpp
    LevelParser.cpp
    LevelParser.hpp
    LevelRepository.cpp
    LevelRepository.hpp
    Paddle.hpp
    PowerUp.hpp
    Rect.hpp
    Vec2.hpp)
add_library(bb::core ALIAS bb_core)

# Headers are included as "core/Vec2.hpp", i.e. relative to src/.
target_include_directories(bb_core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_compile_features(bb_core PUBLIC cxx_std_20)
bb_enable_warnings(bb_core)
