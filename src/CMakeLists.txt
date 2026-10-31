add_subdirectory(core)
add_subdirectory(ui)

if(BB_BUILD_GAME)
    add_subdirectory(app)
endif()
