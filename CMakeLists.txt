cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME NUEDC)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Chassis/BSP/Motor.cpp
        Chassis/BSP/Motor.hpp
        Chassis/BSP/Encode.cpp
        Chassis/BSP/Encode.hpp
        Chassis/APP/Control.cpp
        Chassis/APP/Control.hpp
        Chassis/BSP/RobotHardware.cpp
        Chassis/BSP/RobotHardware.hpp
        Chassis/APP/Serial.cpp
        Chassis/APP/Serial.hpp
        Chassis/Module/PIDcontrol.cpp
        Chassis/Module/PIDcontrol.hpp
        Chassis/APP/Vision.cpp
        Chassis/APP/Vision.hpp
        Chassis/Module/Callback.cpp
        Chassis/Module/Callback.hpp
        Chassis/BSP/MPU6050.cpp
        Chassis/BSP/MPU6050.hpp
        Chassis/Module/Delay.cpp
        Chassis/Module/Delay.hpp
        Chassis/BSP/remote.cpp
        Chassis/BSP/remote.hpp
        Chassis/BSP/uitrasonic.cpp
        Chassis/BSP/uitrasonic.hpp
        Chassis/BSP/OLED.cpp
        Chassis/BSP/OLED.hpp
        Chassis/BSP/OLED_fonts.cpp
        Chassis/BSP/OLED_fonts.hpp
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths

)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
        Chassis/BSP
        Chassis/Module
        Chassis/APP
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
