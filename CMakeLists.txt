cmake_minimum_required(VERSION 3.13.5)

project(NoRegPatch-FalloutNV LANGUAGES C RC)

# Check for 32-bit Windows
if(NOT (WIN32 AND (CMAKE_SIZEOF_VOID_P EQUAL 4)))
    message(FATAL_ERROR "32-bit Windows is the only supported platform!")
endif()

# Find MinHook
set(MINHOOK_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/minhook/include)
if(MINGW)
    set(MINHOOK_LIBRARY ${PROJECT_SOURCE_DIR}/minhook/lib32-mingw/minhook.x32.a)
else()
    message(FATAL_ERROR "Compiler not supported!")
endif()

add_library(NoRegPatch SHARED
    NoRegPatch.c
    version.rc
    GalaxyWrp.def
)

set_target_properties(NoRegPatch PROPERTIES PREFIX "")
set_target_properties(NoRegPatch PROPERTIES OUTPUT_NAME "GalaxyWrp")
set_target_properties(NoRegPatch PROPERTIES SUFFIX ".dll")

target_include_directories(NoRegPatch PRIVATE ${MINHOOK_INCLUDE_DIR})
target_link_libraries(NoRegPatch ${MINHOOK_LIBRARY})
