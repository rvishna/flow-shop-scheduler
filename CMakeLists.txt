cmake_minimum_required(VERSION 3.13)
project(flow_shop_scheduler)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_VERBOSE_MAKEFILE "ON")
if(NOT WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -pedantic")
endif()

find_package(Threads)
find_package(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)
find_package(nlohmann_json REQUIRED)
find_library(TZ_LIBRARY tz)

set(Boost_USE_STATIC_LIBS "ON")
set(Boost_USE_STATIC_RUNTIME "OFF")
set(Boost_ARCHITECTURE "-x64")
find_package(Boost "1.65" REQUIRED COMPONENTS system)
include_directories("${Boost_INCLUDE_DIRS}")
link_directories("${Boost_LIBRARY_DIRS}")

enable_testing()

add_subdirectory(src)
add_subdirectory(tests)
