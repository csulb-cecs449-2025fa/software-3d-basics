# Add source to this project's executable.
add_executable (ClipCoordinates "src/main.cpp" "include/lines.h" "include/triangles.h" "src/lines.cpp" "src/triangles.cpp") 

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(ClipCoordinates PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(ClipCoordinates PRIVATE assimp::assimp)

target_include_directories(ClipCoordinates PUBLIC "./include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ClipCoordinates PROPERTY CXX_STANDARD 20)
endif()