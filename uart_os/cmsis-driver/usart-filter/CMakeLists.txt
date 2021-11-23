cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0053 OLD)
bsp_subprj_dir_2_subprj_name(_PROJECT_NAME_ "${CMAKE_CURRENT_SOURCE_DIR}")
#set(_PROJECT_ ${_PROJECT_NAME_} C CXX ASM)
message( "${MESSAGE_TABS}Folder ${_PROJECT_} ..." )
set(MESSAGE_TABS "${MESSAGE_TABS}\t")

# add this directory to the eclipse source directories
register_eclipse_prj_source_dir("${_PROJECT_NAME_}")
# project(${_PROJECT_})

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
# end fo options.cmake includes from the source and binary dir


# console-usart-filter library
if ("${CMSIS_USART_FILTER_LIBS}" MATCHES "console-usart-filter")
add_library(console-usart-filter STATIC
	console_usart_filter.cpp
)
endif ("${CMSIS_USART_FILTER_LIBS}" MATCHES "console-usart-filter")

# buffering-usart-filter library
if ("${CMSIS_USART_FILTER_LIBS}" MATCHES "buffering-usart-filter")
add_library(buffering-usart-filter STATIC
	buffering_usart_filter.cpp
	
)
endif ("${CMSIS_USART_FILTER_LIBS}" MATCHES "buffering-usart-filter")
	
# bacnetMSTP-usart-filter library
if ("${CMSIS_USART_FILTER_LIBS}" MATCHES "bacnetMSTP-usart-filter")
add_library(bacnetMSTP-usart-filter STATIC
	bacnetMSTP_usart_filter.cpp
)
endif ("${CMSIS_USART_FILTER_LIBS}" MATCHES "bacnetMSTP-usart-filter")

# rs485-usart-filter library
if ("${CMSIS_USART_FILTER_LIBS}" MATCHES "rs485-usart-filter")
add_library(rs485-usart-filter STATIC
	rs485_usart_filter.cpp
)
endif ("${CMSIS_USART_FILTER_LIBS}" MATCHES "rs485-usart-filter")


if(NOT ${MESSAGE_TABS} STREQUAL "")
	STRING(SUBSTRING ${MESSAGE_TABS} 1 -1 MESSAGE_TABS)
endif(NOT ${MESSAGE_TABS} STREQUAL "")
message( "${MESSAGE_TABS}Folder ${_PROJECT_} done.\n" )
