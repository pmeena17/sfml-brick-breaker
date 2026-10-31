# The playable game: SFML adapters + main loop. This is the only directory that needs SFML.
include(FetchContent)

# Build SFML as static libraries (no DLLs to ship) and skip the modules this game does not use.
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
set(SFML_BUILD_AUDIO OFF CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)

# Use an installed SFML 3 if there is one (find_package), otherwise download and build the pinned version.
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG ${BB_SFML_TAG}
    GIT_SHALLOW ON
    EXCLUDE_FROM_ALL
    SYSTEM
    FIND_PACKAGE_ARGS 3 COMPONENTS Graphics)
FetchContent_MakeAvailable(SFML)

add_executable(brick_breaker
    Application.cpp
    Application.hpp
    AssetLocator.cpp
    AssetLocator.hpp
    FontLocator.cpp
    FontLocator.hpp
    SfmlInput.cpp
    SfmlInput.hpp
    SfmlRenderer.cpp
    SfmlRenderer.hpp
    main.cpp)

target_link_libraries(brick_breaker PRIVATE bb::ui bb::core SFML::Graphics)
bb_enable_warnings(brick_breaker)

# Lets the game find ./assets while you develop, even when launched from an IDE with an unusual working directory.
target_compile_definitions(brick_breaker PRIVATE BB_DEFAULT_ASSETS_DIR="${PROJECT_SOURCE_DIR}/assets")

# Copy the assets next to the executable so the build folder is a complete, runnable game.
add_custom_command(TARGET brick_breaker POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${PROJECT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:brick_breaker>/assets)

# Only needed when SFML is built/installed as shared libraries on Windows.
if(WIN32 AND BUILD_SHARED_LIBS)
    add_custom_command(TARGET brick_breaker POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:brick_breaker> $<TARGET_FILE_DIR:brick_breaker>
        COMMAND_EXPAND_LISTS)
endif()

install(TARGETS brick_breaker RUNTIME DESTINATION .)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets DESTINATION .)
