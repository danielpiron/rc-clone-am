
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB UNITTEST_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp)

add_executable(
  unit_tests
  ${UNITTEST_SOURCE}
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
# target_include_directories(unit_tests PUBLIC "${PROJECT_BINARY_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(
  unit_tests
  gtest_main
)
target_compile_options(unit_tests PUBLIC ${CLANG_WARNINGS} -Werror -g)
target_include_directories(unit_tests PRIVATE "${PROJECT_BINARY_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_include_directories(unit_tests SYSTEM PUBLIC "${PROJECT_BINARY_DIR}" ${CMAKE_CURRENT_SOURCE_DIR}/../deps/glm)
target_link_options(unit_tests PUBLIC)

include(GoogleTest)
gtest_discover_tests(unit_tests)