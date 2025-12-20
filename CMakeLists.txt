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

include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)
find_package(OpenSSL REQUIRED)


target_link_libraries(chip8 PRIVATE 
    glfw 
    OpenGL::GL
    OpenSSL::Crypto
)

if (WIN32)
    target_link_libraries(chip8 PRIVATE winmm)
endif()