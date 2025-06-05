# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("Software3DBasics")

# Include sub-projects.
add_subdirectory ("Static2D")
add_subdirectory ("ClipSpace")
add_subdirectory ("Vertex3D")
add_subdirectory ("ViewSpace")
add_subdirectory ("WorldSpace")
add_subdirectory ("LocalSpace")
add_subdirectory ("Matrices")
add_subdirectory ("Assimp")

