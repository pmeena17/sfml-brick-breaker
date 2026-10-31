add_executable(bb_tests
    CoreTests.cpp
    GameSessionTests.cpp
    LevelTests.cpp
    TestFramework.hpp
    TestMain.cpp
    UiTests.cpp)

target_link_libraries(bb_tests PRIVATE bb::ui bb::core)
target_compile_definitions(bb_tests PRIVATE BB_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets")
bb_enable_warnings(bb_tests)

add_test(NAME bb_tests COMMAND bb_tests)
