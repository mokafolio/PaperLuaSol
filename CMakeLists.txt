cmake_minimum_required (VERSION 2.8.11)
project (PaperLuaSol)
set(CMAKE_CXX_FLAGS "-std=c++14")

find_package(Lua 5.3 REQUIRED)
include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include ${LUA_INCLUDE_DIR})
link_directories(/usr/local/lib)

set (PAPERLUADEPS Stick Paper2 pthread ${LUA_LIBRARIES})

set (PAPERLUAINC
PaperLuaSol/PaperLuaSol.hpp
)

install (FILES ${PAPERLUAINC} DESTINATION /usr/local/include/PaperLuaSol)
add_subdirectory (Tests)
