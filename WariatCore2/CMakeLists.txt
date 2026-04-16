cmake_minimum_required(VERSION 3.10)
set(HFRAMEWORK_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hFramework")
set(HSENSORS_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hSensors")
set(HMODULES_PATH "D:/HusarionCore2Tools/dist/HusarionCore2Tools-v1.0.2/HusarionCore2Tools-v1.0.2/hModules")
project(WariatCore2 NONE)

include(${HFRAMEWORK_PATH}/hFramework.cmake)
enable_module(hSensors)
enable_module(hModules)

add_hexecutable(WariatCore2
	main.cpp
	src/ESPInterface.cpp
)
