# Source files
file(GLOB_RECURSE SANDBOX_HEADERS src/*.hpp)
file(GLOB_RECURSE SANDBOX_SOURCES src/*.cpp)

add_executable(Sandbox ${SANDBOX_HEADERS} ${SANDBOX_SOURCES})
target_include_directories(Sandbox PRIVATE src)

# InteropLib
target_include_directories(Sandbox PRIVATE ${CMAKE_SOURCE_DIR}/InteropLib/src)
target_link_libraries(Sandbox InteropLib)