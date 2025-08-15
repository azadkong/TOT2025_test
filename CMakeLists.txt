cmake_minimum_required(VERSION 3.16)
project(GFC_GTest_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Qt (Core only) ----
set(QT_REQUIRED_COMPONENTS Core)
find_package(Qt6 QUIET COMPONENTS ${QT_REQUIRED_COMPONENTS})
if (Qt6_FOUND)
  set(QT_PACKAGE Qt6)
else()
  find_package(Qt5 REQUIRED COMPONENTS ${QT_REQUIRED_COMPONENTS})
  set(QT_PACKAGE Qt5)
endif()
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ---- Project library ----
add_library(gfc_project_lib
  src/gfcparser.cpp
  src/gfcparser.h
  src/expressparser.cpp
  src/expressparser.h
)
target_link_libraries(gfc_project_lib PUBLIC ${QT_PACKAGE}::Core)
target_include_directories(gfc_project_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ===================== GTest (online fetch - ZIP first, short paths) =====================
option(GTEST_FETCH "Fetch Googletest from the internet if not found" ON)
set(GTEST_URL "https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip" CACHE STRING "Release archive URL to fetch (zip/tar.gz). Leave empty to use git.")
set(GTEST_TAG "v1.17.0" CACHE STRING "Googletest git tag/branch if GTEST_URL is empty (e.g. v1.17.0 or main)")

# Use a very short base dir to avoid MAX_PATH issues on Windows + Ninja
# e.g., <build>/fc/{dl,src,bin,st}
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/fc")
file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}")

find_package(GTest QUIET CONFIG)

if (NOT GTest_FOUND)
  if (GTEST_FETCH)
    include(FetchContent)
    set(FETCHCONTENT_QUIET OFF)
    message(STATUS "Fetching Googletest into short dirs under: ${FETCHCONTENT_BASE_DIR}")
    if (GTEST_URL)
      FetchContent_Declare(
        googletest
        URL                 ${GTEST_URL}
        DOWNLOAD_DIR        ${FETCHCONTENT_BASE_DIR}/dl
        SOURCE_DIR          ${FETCHCONTENT_BASE_DIR}/src/gt
        BINARY_DIR          ${FETCHCONTENT_BASE_DIR}/bin/gt
        STAMP_DIR           ${FETCHCONTENT_BASE_DIR}/st/gt
        TLS_VERIFY          ON
        UPDATE_DISCONNECTED OFF
      )
    else()
      FetchContent_Declare(
        googletest
        GIT_REPOSITORY      https://github.com/google/googletest.git
        GIT_TAG             ${GTEST_TAG}
        GIT_SHALLOW         TRUE
        GIT_PROGRESS        TRUE
        DOWNLOAD_DIR        ${FETCHCONTENT_BASE_DIR}/dl
        SOURCE_DIR          ${FETCHCONTENT_BASE_DIR}/src/gt
        BINARY_DIR          ${FETCHCONTENT_BASE_DIR}/bin/gt
        STAMP_DIR           ${FETCHCONTENT_BASE_DIR}/st/gt
        UPDATE_DISCONNECTED OFF
      )
    endif()
    FetchContent_MakeAvailable(googletest)

    if (TARGET gtest AND NOT TARGET GTest::gtest)
      add_library(GTest::gtest ALIAS gtest)
    endif()
    if (TARGET gtest_main AND NOT TARGET GTest::gtest_main)
      add_library(GTest::gtest_main ALIAS gtest_main)
    endif()
  else()
    message(FATAL_ERROR "GTest not found and GTEST_FETCH=OFF. Please install gtest or enable fetching.")
  endif()
endif()
# =================== /GTest (online fetch - ZIP first, short paths) ======================



# ---- Tests ----
enable_testing()

add_executable(gfc_tests
  tests/gfcparser_test.cpp
  tests/expressparser_test.cpp
)
if (MSVC)
  target_compile_options(gfc_tests PRIVATE /EHsc)
endif()
target_link_libraries(gfc_tests PRIVATE gfc_project_lib GTest::gtest GTest::gtest_main ${QT_PACKAGE}::Core)

include(GoogleTest)
gtest_discover_tests(gfc_tests
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
  DISCOVERY_MODE PRE_TEST
)
