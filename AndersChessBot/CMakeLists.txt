file(GLOB_RECURSE ACB_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/AndersChessBot/src/*.cpp)
file(GLOB_RECURSE ACB_C_SOURCES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/AndersChessBot/src/*.c)

add_library(AndersChessBot STATIC ${ACB_SOURCES} ${ACB_C_SOURCES})
target_include_directories(AndersChessBot PUBLIC "${PROJECT_SOURCE_DIR}/AndersChessBot/src")
# target_precompile_headers(LightHouse PRIVATE "${CMAKE_SOURCE_DIR}/LightHouse/src/pch.h")