cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0053 OLD)
bsp_subprj_dir_2_subprj_name(_PROJECT_NAME_ "${CMAKE_CURRENT_SOURCE_DIR}" rtos)
set(_PROJECT_ ${_PROJECT_NAME_} C CXX ASM)
message( "${MESSAGE_TABS}Folder ${_PROJECT_} ..." )
set(MESSAGE_TABS "${MESSAGE_TABS}\t")

# add this directory to the eclipse source directories
register_eclipse_prj_source_dir("${_PROJECT_NAME_}")
project(${_PROJECT_})

# add this directory to the doxygen input directories
register_doxygen_input_dir("${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT ${MESSAGE_TABS} STREQUAL "")
	STRING(SUBSTRING ${MESSAGE_TABS} 1 -1 MESSAGE_TABS)
endif(NOT ${MESSAGE_TABS} STREQUAL "")
message( "${MESSAGE_TABS}Folder ${_PROJECT_} done.\n" )
