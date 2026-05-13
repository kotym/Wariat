idf_component_register(
    SRCS
        "main.cpp"
        "Map.cpp"
        "uart.cpp"
        "../../WariatCommon/ComMap.cpp"
    INCLUDE_DIRS
        "."
        "../../WariatCommon"
)

target_compile_definitions(${COMPONENT_LIB} PRIVATE WARIAT_CODE_ESP)