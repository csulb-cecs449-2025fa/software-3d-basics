﻿# Add source to this project's executable.
add_executable (LocalSpace "src/main.cpp" "include/lines.h" "include/triangles.h" "src/lines.cpp" "src/triangles.cpp") 

find_package(SFML COMPONENTS System Window Graphics CONFIG REQUIRED)
target_link_libraries(LocalSpace PRIVATE SFML::System SFML::Window SFML::Graphics)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(LocalSpace PRIVATE assimp::assimp)

target_include_directories(LocalSpace PUBLIC "./include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LocalSpace PROPERTY CXX_STANDARD 20)
endif()

