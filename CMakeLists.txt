cmake_minimum_required(VERSION 3.14)
project(ChessBot C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Sources ──────────────────────────────────────────────────────────────
set(SOURCES chess.c)

# ── Find SDL2 ────────────────────────────────────────────────────────────
# On Windows (MSYS2/MinGW) the .pc files live under the MSYS2 prefix.
# On Linux/macOS they are found by default via pkg-config.
find_package(PkgConfig REQUIRED)

pkg_check_modules(SDL2 REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Fall back to manual find on Windows if pkg-config missed them
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

# ── Executable ───────────────────────────────────────────────────────────
add_executable(chess-bot ${SOURCES})

# Include dirs
target_include_directories(chess-bot PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

# Link dirs (needed on some platforms)
target_link_directories(chess-bot PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_IMAGE_LIBRARY_DIRS}
)

# Libraries
target_link_libraries(chess-bot PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# On Windows, also link against the implicit Windows libs
if(WIN32)
    target_link_libraries(chess-bot PRIVATE mingw32)
endif()

# ── Copy assets next to the binary at build time ─────────────────────────
# This makes `img/` and `komodo.txt` available next to the .exe / binary
# so you can run directly from the build tree.
add_custom_command(TARGET chess-bot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/img $<TARGET_FILE_DIR:chess-bot>/img
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/komodo.txt $<TARGET_FILE_DIR:chess-bot>/komodo.txt
    COMMENT "Copying img/ and komodo.txt next to binary"
)

# ── Install target (used by CPack / CI scripts) ─────────────────────────
install(TARGETS chess-bot RUNTIME DESTINATION .)

# Collect SDL DLLs on Windows for redistribution
if(WIN32)
    install(CODE "
        include(GetPrerequisites)
        get_prerequisites(\${CMAKE_INSTALL_PREFIX}/chess-bot.exe DEPS 0 0 \"\" \"\")
        foreach(DEP \${DEPS})
            get_filename_component(DEP_NAME \${DEP} NAME)
            if(DEP_NAME MATCHES \"(SDL2|libpng|libjpeg|libtiff|libwebp|zlib)\")
                file(INSTALL DESTINATION \${CMAKE_INSTALL_PREFIX}
                     TYPE SHARED_LIBRARY FILES \${DEP})
                message(STATUS \"Bundled DLL: \${DEP_NAME}\")
            endif()
        endforeach()
    ")
endif()

install(DIRECTORY img/ DESTINATION ./img)
install(FILES komodo.txt DESTINATION .)
