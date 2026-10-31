# bb_enable_warnings(<target>)
# Applies the project's warning level to a target. Kept in one place so every target is treated the same.
function(bb_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8 /w14640 /w14826 /w14905 /w14906 /w14928)
        # <windows.h> defines min/max macros that break std::min/std::max, and many "unsafe" CRT warnings.
        target_compile_definitions(${target} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
        if(BB_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
            -Wold-style-cast -Wnon-virtual-dtor -Woverloaded-virtual -Wdouble-promotion)
        if(BB_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()
