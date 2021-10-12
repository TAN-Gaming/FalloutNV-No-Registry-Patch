cmake_minimum_required(VERSION 3.15)

project(FalloutNV_NoReg LANGUAGES CXX RC)

# Check 32-Bit
if(NOT (CMAKE_SIZEOF_VOID_P EQUAL 4))
  message(FATAL_ERROR "32-bit is only supported.")
endif()

if(MINGW)
  option(BUILD_MINGW_STATIC "Use MinGW static runtime" ON)
endif()

# Find MinHook
set(MINHOOK_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/minhook/include)

if(MINGW)
  set(MINHOOK_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/minhook/lib32-mingw/minhook.x32.a)
endif()

add_library(NoRegPatch SHARED
  NoRegPatch.cpp
  version.rc
  GalaxyWrp.def
)

set_target_properties(NoRegPatch PROPERTIES PREFIX "")
set_target_properties(NoRegPatch PROPERTIES OUTPUT_NAME "GalaxyWrp")
set_target_properties(NoRegPatch PROPERTIES SUFFIX ".dll")

if(BUILD_MINGW_STATIC)
  target_link_options(NoRegPatch PRIVATE -static -static-libgcc -static-libstdc++)
endif()

target_include_directories(NoRegPatch PRIVATE ${MINHOOK_INCLUDE_DIR})

target_link_libraries(NoRegPatch ${MINHOOK_LIBRARY})

