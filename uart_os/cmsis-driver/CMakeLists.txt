cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0053 OLD)
bsp_subprj_dir_2_subprj_name(_PROJECT_NAME_ "${CMAKE_CURRENT_SOURCE_DIR}")
set(_PROJECT_ ${_PROJECT_NAME_} C CXX ASM)
message( "${MESSAGE_TABS}Folder ${_PROJECT_} ..." )
set(MESSAGE_TABS "${MESSAGE_TABS}\t")

# add this directory to the eclipse source directories
register_eclipse_prj_source_dir("${_PROJECT_NAME_}")
project(${_PROJECT_})

# add this directory to the doxygen input directories
register_doxygen_input_dir("${CMAKE_CURRENT_SOURCE_DIR}")


# options.cmake includes from the source and binary dir
include(${CMAKE_CURRENT_SOURCE_DIR}/options.cmake OPTIONAL RESULT_VARIABLE OPTIONAL_INCLUDE_SRC)
include(${CMAKE_CURRENT_BINARY_DIR}/options.cmake OPTIONAL RESULT_VARIABLE OPTIONAL_INCLUDE_BIN)

if (NOT "${OPTIONAL_INCLUDE_SRC}" STREQUAL "NOTFOUND")
message( "${MESSAGE_TABS}Extra include of: ${OPTIONAL_INCLUDE_SRC}" )
endif (NOT "${OPTIONAL_INCLUDE_SRC}" STREQUAL "NOTFOUND")

if (NOT "${OPTIONAL_INCLUDE_BIN}" STREQUAL "NOTFOUND")
message( "${MESSAGE_TABS}Extra include of: ${OPTIONAL_INCLUDE_BIN}" )
endif (NOT "${OPTIONAL_INCLUDE_BIN}" STREQUAL "NOTFOUND")


# driver usart library
if ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-usart")
add_library(cmsis-driver-usart STATIC
	Driver_USART.cpp
)
endif ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-usart")

# driver i2c library
if ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-i2c")
add_library(cmsis-driver-i2c STATIC
	Driver_I2C.cpp
)
endif ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-i2c")

# driver flash library
if ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-flash")
add_library(cmsis-driver-flash STATIC
	Driver_Flash.cpp
#	Driver_Flash_SPI.cpp
)
endif ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-flash")

# driver usb device library
if ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-usbd")
add_library(cmsis-driver-usbd STATIC
	Driver_USBD.cpp
)
endif ("${CMSIS_DRIVER_LIBS}" MATCHES "cmsis-driver-usbd")


if(NOT ${MESSAGE_TABS} STREQUAL "")
	STRING(SUBSTRING ${MESSAGE_TABS} 1 -1 MESSAGE_TABS)
endif(NOT ${MESSAGE_TABS} STREQUAL "")
message( "${MESSAGE_TABS}Folder ${_PROJECT_} done.\n" )
