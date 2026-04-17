cmake_minimum_required(VERSION 3.10)
set(HFRAMEWORK_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hFramework")
set(HSENSORS_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hSensors")
set(HMODULES_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hModules")
project(WariatCore2 NONE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(WARIAT_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
set(WARIAT_COMMON_PATH "${WARIAT_REPO_ROOT}/WariatCommon")

if(NOT EXISTS "${WARIAT_COMMON_PATH}/CommunicationProtocol.hpp")
	message(FATAL_ERROR "WariatCommon not found at: ${WARIAT_COMMON_PATH}")
endif()

file(GLOB WARIAT_COMMON_SOURCES CONFIGURE_DEPENDS "${WARIAT_COMMON_PATH}/*.cpp")

include(${HFRAMEWORK_PATH}/hFramework.cmake)
enable_module(hSensors)
enable_module(hModules)

include_directories(
	${WARIAT_REPO_ROOT}
	${WARIAT_COMMON_PATH}
)

add_hexecutable(WariatCore2
	main.cpp
	src/ESPInterface.cpp
	${WARIAT_COMMON_SOURCES}
)

# hFramework hardcodes -std=c++11 in global flags, so enforce C++20 on this target.
set_target_properties(WariatCore2.elf PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
target_compile_options(WariatCore2.elf PRIVATE -std=gnu++20)
