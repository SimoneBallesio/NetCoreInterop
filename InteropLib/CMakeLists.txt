# Source files
file(GLOB_RECURSE INTEROP_LIB_HEADERS src/*.hpp)
file(GLOB_RECURSE INTEROP_LIB_SOURCES src/*.cpp)

add_library(InteropLib SHARED ${INTEROP_LIB_HEADERS} ${INTEROP_LIB_SOURCES})
set_target_properties(InteropLib PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(InteropLib PRIVATE src)

# Definitions
target_compile_definitions(InteropLib PRIVATE INTEROP_EXPORT)

# hostfxr (.NET Core 9)
target_include_directories(InteropLib PRIVATE vendor/dotnet/include)