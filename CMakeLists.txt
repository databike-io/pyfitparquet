cmake_minimum_required(VERSION 3.15)

# set the project name
project(fit-ingest)

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

# Build fitCppSDK library
file(GLOB SDKSRC "${FITSDK_VERSION}/cpp/*.cpp")
add_library(fitSDK ${SDKSRC})
target_include_directories(fitSDK PUBLIC "${FITSDK_VERSION}/cpp")

# Build decoder executable
add_executable(decoder decode.cpp)
target_link_libraries(decoder PRIVATE fitSDK)

# Build fitparquet executable
add_executable(fitparquet fitparquet.cpp)
if (ARROW_LINK_SHARED)
    # Prefer shared linkage but use static if shared build is deactivated
    target_link_libraries(fitparquet PRIVATE arrow_shared parquet_shared fitSDK)
else()
    # ** NOTE: static linkage does NOT work :( **
    target_link_libraries(fitparquet PRIVATE arrow_static parquet_static fitSDK)
    # ** NOTE: static linkage does NOT work :( **
endif()

