find_package(GTest CONFIG REQUIRED)

add_executable(software_3d_raster_tests
    src/framebuffer_tests.cpp
    src/line_tests.cpp
    src/triangle_wireframe_tests.cpp
)

target_link_libraries(software_3d_raster_tests PRIVATE common_drawing GTest::gtest_main)
target_include_directories(software_3d_raster_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_compile_features(software_3d_raster_tests PRIVATE cxx_std_20)

include(GoogleTest)
gtest_discover_tests(software_3d_raster_tests)
