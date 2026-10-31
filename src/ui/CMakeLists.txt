# Screens, input events and the renderer interface. Depends on bb_core only - still no graphics library.
add_library(bb_ui STATIC
    Button.cpp
    Button.hpp
    Color.hpp
    GameplayScreen.cpp
    GameplayScreen.hpp
    IRenderer.hpp
    InputEvent.hpp
    InstructionsScreen.cpp
    InstructionsScreen.hpp
    MenuScreen.cpp
    MenuScreen.hpp
    ResultScreen.cpp
    ResultScreen.hpp
    Screen.hpp
    ScreenCommand.hpp
    ScreenNavigator.cpp
    ScreenNavigator.hpp
    Theme.hpp)
add_library(bb::ui ALIAS bb_ui)

target_link_libraries(bb_ui PUBLIC bb::core)
bb_enable_warnings(bb_ui)
