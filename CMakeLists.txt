cmake_minimum_required(VERSION 3.16)
project(chip8 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


file(GLOB VENDOR_INCLUDE "${CMAKE_SOURCE_DIR}/vendor/*/include")
file(GLOB_RECURSE SOURCES vendor/*.c vendor/*.cpp src/*.cpp src/*.c)
add_executable(chip8 ${SOURCES})

target_include_directories(chip8 PRIVATE 
    ${CMAKE_SOURCE_DIR}/include 
    ${VENDOR_INCLUDE}
)
target_link_directories(chip8 PRIVATE ${CMAKE_SOURCE_DIR}/lib)

if (WIN32)
    target_link_libraries(chip8 PRIVATE glfw3dll opengl32 crypto winmm)
else()
    target_link_libraries(chip8 PRIVATE glfw3dll opengl32 crypto)
endif()