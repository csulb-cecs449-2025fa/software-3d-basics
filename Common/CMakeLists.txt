add_library(common_drawing
    STATIC 
    src/lines.cpp src/triangles.cpp
)


find_package(SFML COMPONENTS System Window Graphics CONFIG REQUIRED)
target_link_libraries(common_drawing PRIVATE SFML::System SFML::Window SFML::Graphics)

target_include_directories(common_drawing PUBLIC "./include")

target_compile_features(common_drawing PUBLIC cxx_std_20)