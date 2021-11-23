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

add_subdirectory(../cmsis-rtos-ext ./rtos/cmsis-rtos-ext)
add_subdirectory(../cmsis-rtos ./cmsis-rtos)
add_subdirectory(../c++ ./c++)

# options.cmake includes from the source and binary dir
include(${CMAKE_CURRENT_SOURCE_DIR}/options.cmake OPTIONAL RESULT_VARIABLE OPTIONAL_INCLUDE_SRC)
include(${CMAKE_CURRENT_BINARY_DIR}/options.cmake OPTIONAL RESULT_VARIABLE OPTIONAL_INCLUDE_BIN)

if (NOT "${OPTIONAL_INCLUDE_SRC}" STREQUAL "NOTFOUND")
message( "${MESSAGE_TABS}Extra include of: ${OPTIONAL_INCLUDE_SRC}" )
endif (NOT "${OPTIONAL_INCLUDE_SRC}" STREQUAL "NOTFOUND")

if (NOT "${OPTIONAL_INCLUDE_BIN}" STREQUAL "NOTFOUND")
message( "${MESSAGE_TABS}Extra include of: ${OPTIONAL_INCLUDE_BIN}" )
endif (NOT "${OPTIONAL_INCLUDE_BIN}" STREQUAL "NOTFOUND")
# end fo options.cmake includes from the source and binary dir


add_library(${TARGET_RTOS} STATIC
	../cmsis-rtos-ext/osCom.cpp
	../cmsis-rtos-ext/osAdc.cpp
	../cmsis-rtos-ext/osBi.cpp
	../cmsis-rtos-ext/osFlash.cpp
	cmsis_os_NoRTOS.cpp
)

if(NOT ${MESSAGE_TABS} STREQUAL "")
	STRING(SUBSTRING ${MESSAGE_TABS} 1 -1 MESSAGE_TABS)
endif(NOT ${MESSAGE_TABS} STREQUAL "")
message( "${MESSAGE_TABS}Folder ${_PROJECT_} done.\n" )
