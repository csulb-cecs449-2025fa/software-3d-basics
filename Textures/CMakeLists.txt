# Add source to this project's executable.
add_executable (Textures "src/main.cpp" "include/Mesh.h" "include/StbImage.h" "src/StbImage.cpp") 

find_package(SFML COMPONENTS System Window Graphics CONFIG REQUIRED)
target_link_libraries(Textures PRIVATE SFML::System SFML::Window SFML::Graphics)

find_package(assimp CONFIG REQUIRED)
target_link_libraries(Textures PRIVATE assimp::assimp)
target_link_libraries(Textures PRIVATE common_drawing)


target_include_directories(Textures PUBLIC "./include")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Textures PROPERTY CXX_STANDARD 20)
endif()


add_custom_target(copymodels_textures
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/models
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/models ${CMAKE_CURRENT_BINARY_DIR}/models
        COMMENT "copying ${CMAKE_SOURCE_DIR}/models to ${CMAKE_CURRENT_BINARY_DIR}/models"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
add_dependencies(Textures copymodels_textures)
