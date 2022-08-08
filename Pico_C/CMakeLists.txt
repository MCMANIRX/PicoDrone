cmake_minimum_required(VERSION 3.12)

#build fcns
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
#include($ENV{PICO_EXTRAS_PATH}/external/pico_extras_import.cmake)
include(pico_extras_import.cmake)




project(Drone C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)



pico_sdk_init()



#添加编译子目录
add_subdirectory(lib/Config)
add_subdirectory(lib/LCD)
add_subdirectory(lib/Fonts)
add_subdirectory(lib/GUI)
add_subdirectory(lib/3D)
add_subdirectory(lib/sprite)




#添加头文件目录
include_directories(./lib/Config)
include_directories(./lib/GUI)
include_directories(./lib/LCD)
include_directories(./lib/3D)




add_executable(${PROJECT_NAME}
   # raw.c
    frame_test.c
   # frame_test.h
    waveshare_frame.c
    barometer.c
    run_motors.c
  #temp.c

    #vga_out.c
    # ./lib/GUI/GUI_Paint.c
    

    #ErinLED.c
    #runMotors.c
    
    )




# make uf2
pico_add_extra_outputs(${PROJECT_NAME})

#link to pico stdlib


target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_pwm
    LCD
    GUI
    Fonts 
    Config
    3D
    
    
    
)


pico_enable_stdio_usb(Drone 1)
pico_enable_stdio_uart(Drone 0)