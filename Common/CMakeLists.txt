add_library(common_drawing
    STATIC
    src/framebuffer.cpp
    src/lines.cpp
    src/triangles.cpp
)

find_package(glm CONFIG REQUIRED)
target_link_libraries(common_drawing PUBLIC glm::glm-header-only)

target_include_directories(common_drawing PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_features(common_drawing PUBLIC cxx_std_20)
