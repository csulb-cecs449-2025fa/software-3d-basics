# Add source to this project's executable.
add_executable (Vertex3D "src/main.cpp" "include/lines.h" "include/triangles.h" "src/lines.cpp" "src/triangles.cpp") 

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(Vertex3D PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Vertex3D PRIVATE assimp::assimp)

target_include_directories(Vertex3D PUBLIC "./include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Vertex3D PROPERTY CXX_STANDARD 20)
endif()