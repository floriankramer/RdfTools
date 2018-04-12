cmake_minimum_required(VERSION 3.7)

project(NtFilters)

add_library(NtParser NtParser.h NtParser.cpp)

add_executable(extractSubset ExtractSubset.cpp)
add_executable(filterLanguages FilterLanguages.cpp)

target_link_libraries(extractSubset NtParser)
target_link_libraries(filterLanguages NtParser)
