cmake_minimum_required(VERSION 3.15)

# Set the project name
project(fit-ingest)

# Break if Conda not setup/activated
if(NOT DEFINED ENV{CONDA_DEFAULT_ENV})
    message( FATAL_ERROR "(Mini)Conda enviroment required" )
elseif(NOT $ENV{CONDA_DEFAULT_ENV} STREQUAL "databike")
    message( FATAL_ERROR "Conda environment: 'databike' must be activated" )
endif()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FitSDK version number
set(FITSDK_VERSION FitCppSDK_21.40.00)

# Import Apache Arrow build configuration
set(Arrow_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/arrow")
set(Parquet_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/arrow")
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)

# Import needed Boost libraries
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS filesystem REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
# Set fvisibility to resolve ld: visibility warning (conda boost libs)

# Build fitCppSDK library
file(GLOB SDKSRC "${FITSDK_VERSION}/cpp/*.cpp")
add_library(fitSDK ${SDKSRC})
target_include_directories(fitSDK PUBLIC "${FITSDK_VERSION}/cpp")

# Build decoder executable
add_executable(decoder decode.cpp)
target_link_libraries(decoder PRIVATE fitSDK)

# Build fitparquet executable
add_executable(fitparquet fitparquet.cc)
if (ARROW_LINK_SHARED)
    # Prefer shared linkage but use static if shared build is deactivated
    target_link_libraries(fitparquet PRIVATE arrow_shared parquet_shared Boost::filesystem fitSDK)
else()
    # ** NOTE: static linkage does NOT work :( **
    target_link_libraries(fitparquet PRIVATE arrow_static parquet_static Boost::filesystem fitSDK)
    # ** NOTE: static linkage does NOT work :( **
endif()
