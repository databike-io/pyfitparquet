cmake_minimum_required(VERSION 3.15)

# set the project name
project(fit-ingest)

# FitSDK version number
set(FITSDK_VERSION FitCppSDK_21.40.00)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# fitCppSDK library
file(GLOB SDKSRC "${FITSDK_VERSION}/cpp/*.cpp")
add_library(fitSDK ${SDKSRC})

# add the executable
add_executable(decoder decode.cpp)
target_link_libraries(decoder PUBLIC fitSDK)

# fitCppSDK includes
target_include_directories(fitSDK PUBLIC "${FITSDK_VERSION}/cpp")
