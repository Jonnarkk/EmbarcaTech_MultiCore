# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================


cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PICO_BOARD pico_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)
project(DoisCores C CXX ASM)
pico_sdk_init()

add_executable(${PROJECT_NAME}  
        # Pisca_LEDs_2Cores.c 
        ${CMAKE_CURRENT_LIST_DIR}/libs/ssd1306.c
        ${CMAKE_CURRENT_LIST_DIR}/libs/bmp280.c
        #${CMAKE_CURRENT_LIST_DIR}/libs/mpu6050.c
        # Prints_2Cores.c
        # ADC_2Cores.c
        ADC_2Cores_IRQ.c
        )

target_link_libraries(${PROJECT_NAME} 
        pico_stdlib 
        pico_multicore
        hardware_adc
        hardware_i2c
        )

# Adiciona os diretórios de inclusão para o compilador encontrar os .h
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/libs      # Diretório das suas bibliotecas (ssd1306.h, etc.)
)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})



