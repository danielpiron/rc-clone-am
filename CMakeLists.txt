cmake_minimum_required(VERSION 3.0.0)
project(rc_clone_am VERSION 0.1.0)

# Warnings flags borrowed from Jason Turner's cpp_starter_project repo
# https://github.com/lefticus/cpp_starter_project/blob/master/cmake/CompilerWarnings.cmake
#
set(CLANG_WARNINGS
    -Wall
    -Wextra # reasonable and standard
    -Wshadow # warn the user if a variable declaration shadows one from a parent context
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
                    # catch hard to track down memory errors
    # -Wold-style-cast # warn for c-style casts
    -Wcast-align # warn for potential performance problem casts
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual function
    -Wpedantic # warn if non-standard C++ is used
    -Wconversion # warn on type conversions that may lose data
    -Wsign-conversion # warn on sign conversions
    -Wnull-dereference # warn if a null dereference is detected
    -Wdouble-promotion # warn if float is implicit promoted to double
    -Wformat=2 # warn on security issues around functions that format output (ie printf)
)

if(MSVC)
    set(COMPILER_FLAGS /W4)
else()
    set(COMPILER_FLAGS ${CLANG_WARNINGS} -g -Werror)
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(CTest)
enable_testing()

add_subdirectory(deps/glfw)
add_subdirectory(deps/glad)

if(MSVC)
    add_subdirectory(deps/zlib)
    set(PNG_BUILD_ZLIB ON)
    set(ZLIB_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/zlib ${PROJECT_BINARY_DIR}/deps/zlib)
    set(ZLIB_LIBRARIES ${PROJECT_BINARY_DIR}/deps/zlib/Debug/zlibstaticd.lib)
endif()

add_subdirectory(deps/libpng)

add_subdirectory(tests)

add_executable(gl_again ${PLAYER_SOURCE} src/main.cpp)
target_compile_options(gl_again PUBLIC ${COMPILER_FLAGS})
target_link_options(gl_again PUBLIC ${LINKER_FLAGS})
target_link_libraries(gl_again glfw glad png_static ${GLFW_LIBRARIES})
target_include_directories(gl_again PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(gl_again SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include)
target_include_directories(gl_again SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include)
target_include_directories(gl_again SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/glm)
target_include_directories(gl_again SYSTEM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/deps/libpng)
target_include_directories(gl_again SYSTEM PUBLIC ${PROJECT_BINARY_DIR}/deps/libpng)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/fragment.glsl fragment.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/vertex.glsl vertex.glsl COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/rc-truck.obj rc-truck.obj COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/tree.obj tree.obj COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/track_segments.obj track_segments.obj COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/ImphenziaPalette01.png ImphenziaPalette01.png COPYONLY)