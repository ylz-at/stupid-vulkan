cmake_minimum_required(VERSION 3.17)

set(Recipe_Name "Learning_Vulkan")
project(${Recipe_Name})
message(STATUS "Attempting auto locate Vulkan using CMake......")
find_package(Vulkan REQUIRED)

if (NOT ${Vulkan_INCLUDE_DIRS} STREQUAL "")
    set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
    STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})
endif()

if (NOT Vulkan_FOUND)
    message(STATUS "Failed to locate Vulkan SDK, retrying again...")
    if (EXISTS "${VULKAN_PATH}")
        message(STATUS "Successfully located the Vulkan SDK: ${VULKAN_PATH}")
    else()
        message("Error: Unable to locate Vulkan SDK'")
        message("and specify manually path using 'VULKAN_SDK' and 'VULKAN_VERSION' variables in the CMakeLists.txt")
        return()
    endif()
endif()

option(BUILD_SPV_ON_COMPILE_TIME "BUILD_SPV_ON_COMPILE_TIME" OFF)

add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
set(VULKAN_LINK_LIST "vulkan-1")
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    include_directories(AFTER ${VULKAN_PATH}/Include)
    link_directories(${VULKAN_PATH}/Bin;${VULKAN_PATH}/Lib;)
    if(BUILD_SPV_ON_COMPILE_TIME)

        # Preprocessor  flag allows the solution to use glslang library functions
        add_definitions(-DAUTO_COMPILE_GLSL_TO_SPV)

        #GLSL - use Vulkan SDK's glslang library for compling GLSL to SPV
        # This does not require offline coversion of GLSL shader to SPIR-V(.spv) form
        set(GLSLANGDIR "${VULKAN_PATH}/glslang")
        get_filename_component(GLSLANG_PREFIX "${GLSLANGDIR}" ABSOLUTE)
        if(NOT EXISTS ${GLSLANG_PREFIX})
            message(FATAL_ERROR "Necessary glslang components do not exist: " ${GLSLANG_PREFIX})
        endif()
        include_directories( ${GLSLANG_PREFIX} )

        # If compiling GLSL to SPV using we need the following libraries
        set(GLSLANG_LIBS SPIRV glslang OGLCompiler OSDependent HLSL)

        # Generate the list of files to link, per flavor.
        foreach(x ${GLSLANG_LIBS})
            list(APPEND VULKAN_LIB_LIST debug ${x}d optimized ${x})
        endforeach()

        # Note: While configuring CMake for glslang we created the
        # binaries in a "build" folder inside ${VULKAN_PATH}/glslang.
        # Therefore, you must edit the below lines for your custorm
        # path like <Your binary path>/OGLCompilersDLL, <Your binary path>/OSDependent/Windows
        link_directories(${VULKAN_PATH}/glslang/build/OGLCompilersDLL )
        link_directories(${VULKAN_PATH}/glslang/build/glslang/OSDependent/Windows)
        link_directories(${VULKAN_PATH}/glslang/build/glslang)
        link_directories(${VULKAN_PATH}/glslang/build/SPIRV)
        link_directories(${VULKAN_PATH}/glslang/build/hlsl)
    endif()
endif()

if (WIN32)
    source_group("include" REGULAR_EXPRESSION "include/*")
    source_group("source" REGULAR_EXPRESSION "source/*")
endif(WIN32)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
file(GLOB_RECURSE CPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp)
file(GLOB_RECURSE HPP_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/*.*)

add_executable(${Recipe_Name} ${CPP_FILES} ${HPP_FILES})

target_link_libraries(${Recipe_Name} ${VULKAN_LIB_LIST} Vulkan::Vulkan)

add_subdirectory(vendor/glm)
target_link_libraries(${Recipe_Name} glm::glm)

set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_CURRENT_SOURCE_DIR}/binaries)
set_property(TARGET ${Recipe_Name} PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_SOURCE_DIR}/binaries)

set_property(TARGET ${Recipe_Name} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${Recipe_Name} PROPERTY CXX_STANDARD_REQUIRED ON)

set_property(TARGET ${Recipe_Name} PROPERTY C_STANDARD 99)
set_property(TARGET ${Recipe_Name} PROPERTY C_STANDARD_REQUIRED ON)

