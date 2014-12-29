cmake_minimum_required(VERSION 3.0)

set (PROJECT popss)

project(${PROJECT})

# Use our modified FindSDL2* modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${popss_SOURCE_DIR}/cmake")

file(GLOB_RECURSE POPSS_SOURCES "../src/*.cpp" "../src/*.h" "../src/*.hpp")

set(CMAKE_CXX_FLAGS "-std=c++11 -O2")
set(EXTRA_LIBS "")

if (APPLE)

    find_package(OpenGL REQUIRED)

    set(EXTRA_LIBS ${EXTRA_LIBS} "-framework Cocoa")

    #include(FindPkgConfig)
    #pkg_search_module(SDL2 REQUIRED sdl2)
    find_package(SDL2 REQUIRED)
    include_directories(${SDL2_INCLUDE_DIR})

endif (APPLE)

include_directories("../lib/")

add_executable(
    ${PROJECT} 
    #MACOSX_BUNDLE
    ${POPSS_SOURCES}
)

target_link_libraries(
    ${PROJECT}
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARY}
    ${EXTRA_LIBS}
)

INSTALL(TARGETS ${PROJECT} DESTINATION ${popss_SOURCE_DIR}/../build)
