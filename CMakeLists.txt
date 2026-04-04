cmake_minimum_required(VERSION 3.14)
project(ChessBot C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Sources ──────────────────────────────────────────────────────────────
set(SOURCES bot.c)

# ── Find SDL2 ────────────────────────────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2       REQUIRED sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

# Fall back to manual find on Windows if pkg-config missed them
if(NOT SDL2_FOUND)
    find_package(SDL2 REQUIRED)
endif()

# ── Executable ───────────────────────────────────────────────────────────
add_executable(chess-bot ${SOURCES})

target_include_directories(chess-bot PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
)

target_link_directories(chess-bot PRIVATE
    ${SDL2_LIBRARY_DIRS}
    ${SDL2_IMAGE_LIBRARY_DIRS}
)

target_link_libraries(chess-bot PRIVATE
    ${SDL2_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
)

# ── macOS: embed rpath so the binary finds bundled dylibs next to itself ─
if(APPLE)
    set_target_properties(chess-bot PROPERTIES
        INSTALL_RPATH "@executable_path"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# ── Linux: embed rpath so the binary finds bundled .so files next to itself
if(UNIX AND NOT APPLE)
    set_target_properties(chess-bot PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# ── Copy assets next to the binary at build time ─────────────────────────
add_custom_command(TARGET chess-bot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/img $<TARGET_FILE_DIR:chess-bot>/img
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/komodo.txt $<TARGET_FILE_DIR:chess-bot>/komodo.txt
    COMMENT "Copying img/ and komodo.txt next to binary"
)

# ── Install target ───────────────────────────────────────────────────────
install(TARGETS chess-bot RUNTIME DESTINATION .)
install(DIRECTORY img/ DESTINATION ./img)
install(FILES komodo.txt DESTINATION .)
