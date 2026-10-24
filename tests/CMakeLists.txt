cmake_minimum_required( VERSION 3.28 FATAL_ERROR )

project( fixed_vector_tests
	VERSION 0.1.0
	DESCRIPTION ""
	LANGUAGES CXX
)

set( CMAKE_AUTOMOC ON )
set( CMAKE_AUTORCC ON )
set( CMAKE_AUTOUIC ON )

include( FetchContent )

FetchContent_Declare( Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
    EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable( Catch2 )

add_executable( ${PROJECT_NAME}
    src/test_fixed_vector.cpp
)

target_include_directories( ${PROJECT_NAME} PRIVATE
    ../src/
)

target_link_libraries( ${PROJECT_NAME} PRIVATE
    Catch2::Catch2WithMain
    fixed_vector
)

list( APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras )
include( CTest )
include( Catch )

catch_discover_tests( ${PROJECT_NAME} )
