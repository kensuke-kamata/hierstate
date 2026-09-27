cmake_minimum_required(VERSION 3.28)
project(hierstate LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

add_library(hierstate INTERFACE)
add_library(hierstate::hierstate ALIAS hierstate)
target_compile_features(hierstate INTERFACE cxx_std_20)
target_include_directories(hierstate INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(HIERSTATE_PACKAGE_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/hierstate")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/hierstateConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/hierstateConfig.cmake"
    INSTALL_DESTINATION "${HIERSTATE_PACKAGE_DIR}"
)

install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
install(TARGETS hierstate EXPORT hierstateTargets)
install(EXPORT hierstateTargets
    FILE hierstateTargets.cmake
    NAMESPACE hierstate::
    DESTINATION "${HIERSTATE_PACKAGE_DIR}"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/hierstateConfig.cmake"
    DESTINATION "${HIERSTATE_PACKAGE_DIR}"
)

if(PROJECT_IS_TOP_LEVEL)
    add_subdirectory(examples)
    include(CTest)

    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif()
endif()
