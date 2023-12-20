cmake_minimum_required(VERSION 3.11) # FetchContent is available in 3.11+
project(DomBalls)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS    "${CMAKE_CXX_FLAGS} -march=native -mcpu=native -mtune=native")   # binary is optimized, but machine dependent


# raylib
find_package(raylib 3.0.0 QUIET)
if (NOT raylib_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 7ef114d1da2c34a70bba5442497103441647d8f3
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(CMAKE_C_FLAGS  "-Wno-stringop-overflow -Wno-error -Wno-error=implicit-function-declaration -I/opt/vc/include" CACHE STRING "" FORCE)
    set(PLATFORM       "Raspberry Pi"                                                                                 CACHE STRING "" FORCE)
    set(SUPPORT_SSH_KEYBOARD_RPI ON  CACHE BOOL "" FORCE)
    set(BUILD_EXAMPLES           OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES              OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING            OFF CACHE BOOL "" FORCE)
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

# raylib-cpp
find_package(raylib-cpp 0.3.0 QUIET)
if (NOT raylib-cpp_FOUND)
  include(FetchContent)
  FetchContent_Declare(
    raylib-cpp
    GIT_REPOSITORY https://github.com/RobLoach/raylib-cpp.git
    GIT_TAG 64c39d6d6b1ab2f68cb3669b0a3ff57857d5cdf7
  )
  FetchContent_GetProperties(raylib-cpp)
  if (NOT raylib-cpp_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib-cpp)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING  OFF CACHE BOOL "" FORCE)
    add_subdirectory(${raylib-cpp_SOURCE_DIR} ${raylib-cpp_BINARY_DIR})
  endif()
endif()

# This is the main part:
set(PROJECT_NAME DomBalls)
set(PROJECT_SOURCES DomBalls.cpp)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
#set(raylib_VERBOSE 1)
#target_link_libraries(${PROJECT_NAME} PUBLIC raylib raylib-cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC raylib raylib-cpp pthread m "-L/opt/vc/lib/" libbrcmGLESv2.so libbrcmEGL.so libbcm_host.so libvchiq_arm.so libvcos.so)
# That's it! You should have an example executable that you can run. Have fun!
