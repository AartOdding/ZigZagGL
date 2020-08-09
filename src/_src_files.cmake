cmake_minimum_required(VERSION 3.0)

include("${CMAKE_CURRENT_LIST_DIR}/app/_app_files.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/gui/_gui_files.cmake")


set(src_files
    "${app_files}"
    "${gui_files}"

    "${CMAKE_CURRENT_LIST_DIR}/Application.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/Application.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp" 
    "${CMAKE_CURRENT_LIST_DIR}/types.hpp"
)