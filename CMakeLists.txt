cmake_minimum_required(VERSION 3.10)
include(${CMAKE_SOURCE_DIR}/../scripts/CMakeSettings.txt)

project(dummy_output VERSION 0.0.1 DESCRIPTION "dummyaudio output library for Retrovert")

include_directories("retrovert_api/c" "tinycthread")

add_library(dummy_output SHARED "dummy_output.c" "tinycthread/tinycthread.c") 

set_custom_library_extension(dummy_output)
