cmake_minimum_required(VERSION 3.10)

# Enable vcpkg manifest mode
set(VCPKG_MANIFEST_MODE ON)

project(TaxIdleGame VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# MSVC-specific flags
if(MSVC)
    # Release optimizations
    add_compile_options(
        $<$<CONFIG:Release>:/O2>           # Maximum optimization
        $<$<CONFIG:Release>:/GL>           # Whole program optimization
        $<$<CONFIG:Release>:/DNDEBUG>      # Define NDEBUG
    )
    
    # Debug flags
    add_compile_options(
        $<$<CONFIG:Debug>:/Od>             # No optimization
        $<$<CONFIG:Debug>:/Zi>             # Debug info
        $<$<CONFIG:Debug>:/RTC1>           # Runtime checks
    )
    
    # Link-time code generation for Release
    add_link_options(
        $<$<CONFIG:Release>:/LTCG>         # Link-time code generation
    )
endif()

# Find SDL2
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

add_executable(TaxIdleGame
    src/main.cpp
    src/Game.cpp
    include/Game.h
)

target_include_directories(TaxIdleGame
    PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(TaxIdleGame
    PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

# Copy DLLs to output directory for convenience
if(WIN32)
    # This makes running from Visual Studio easier
    add_custom_command(TARGET TaxIdleGame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying SDL2 DLLs..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:TaxIdleGame>
            $<TARGET_FILE_DIR:TaxIdleGame>
        COMMAND_EXPAND_LISTS
    )
endif()

# Set TaxIdleGame as startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT TaxIdleGame)

# Set working directory for Visual Studio debugging
set_target_properties(TaxIdleGame PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
)
