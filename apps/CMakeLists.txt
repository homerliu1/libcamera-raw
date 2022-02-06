cmake_minimum_required(VERSION 3.6)

project(libcamera-raw)
add_executable(libcamera-raw libcamera_raw.cpp)
target_link_libraries(libcamera-raw libcamera_app encoders  ${OpenCV_LIBS})

set(EXECUTABLES libcamera-raw)

set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR})
install(TARGETS ${EXECUTABLES} RUNTIME DESTINATION bin)
