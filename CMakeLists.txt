cmake_minimum_required(VERSION 3.20)
project(GFC_GTest_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect Qt (Core only). Prefer Qt6, fallback to Qt5.
set(QT_REQUIRED_COMPONENTS Core)
find_package(Qt6 QUIET COMPONENTS ${QT_REQUIRED_COMPONENTS})
if (NOT Qt6_FOUND)
  find_package(Qt5 REQUIRED COMPONENTS ${QT_REQUIRED_COMPONENTS})
  set(QT_PACKAGE Qt5)
else()
  set(QT_PACKAGE Qt6)
endif()

# GoogleTest via FetchContent
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# On MSVC: use the same runtime as the parent project
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

# ---- Your project's library (built from src/*.cpp) ----
file(GLOB PROJECT_SRC CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(gfc_project_lib ${PROJECT_SRC})
target_include_directories(gfc_project_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(gfc_project_lib PUBLIC ${QT_PACKAGE}::Core)

# ---- Tests ----
add_executable(gfc_tests
  tests/gfcparser_test.cpp
  tests/expressparser_test.cpp
)
target_link_libraries(gfc_tests
  gfc_project_lib
  GTest::gtest_main
  ${QT_PACKAGE}::Core
)
include(GoogleTest)
gtest_discover_tests(gfc_tests)
