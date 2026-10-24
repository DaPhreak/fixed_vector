cmake_minimum_required( VERSION 3.28 FATAL_ERROR )

project( fixed_vector
    VERSION 0.1.0
    DESCRIPTION ""
    LANGUAGES CXX
)

set( HEADERS
    include/fixed_vector/fixed_vector.h
)

add_library( ${PROJECT_NAME} INTERFACE
    ${HEADERS}
)

target_compile_features( ${PROJECT_NAME} 
    INTERFACE 
        cxx_std_17
)

target_include_directories( ${PROJECT_NAME} 
    INTERFACE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_sources( ${PROJECT_NAME} 
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/fixed_vector/fixed_vector.natvis>
)

add_subdirectory( tests )