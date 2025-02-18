# Add source to this project's executable.
add_executable (Matrices "src/main.cpp" "include/lines.h" "include/triangles.h" "src/lines.cpp" "src/triangles.cpp"  "include/Mesh.h") 

find_package(SFML COMPONENTS system window graphics CONFIG REQUIRED)
target_link_libraries(Matrices PRIVATE sfml-system sfml-network sfml-graphics sfml-window)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Matrices PRIVATE assimp::assimp)

target_include_directories(Matrices PUBLIC "./include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Matrices PROPERTY CXX_STANDARD 20)
endif()


add_custom_target(copymodels_matrices
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/models
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${CMAKE_CURRENT_BINARY_DIR}/models
        COMMENT "copying ${CMAKE_SOURCE_DIR}/models to ${CMAKE_CURRENT_BINARY_DIR}/models"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_dependencies(Matrices copymodels_matrices)