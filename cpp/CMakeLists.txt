cmake_minimum_required(VERSION 3.15)
# Supports dynamic linking from conda env only

# Set project name and lang: C++
project(fit-ingest LANGUAGES CXX)

# Break if Conda not setup/activated
if(NOT DEFINED ENV{CONDA_DEFAULT_ENV})
    message( FATAL_ERROR "(Mini)Conda enviroment required" )
elseif(NOT $ENV{CONDA_DEFAULT_ENV} STREQUAL "pyfitparquet")
    message( FATAL_ERROR "Conda environment: 'pyfitparquet' must be activated" )
endif()

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FitSDK version
set(FITSDK_VERSION FitCppSDK_21.40.00)

# Import Apache build configs
set(Arrow_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/arrow")
set(Parquet_DIR "${CMAKE_INSTALL_PREFIX}/lib/cmake/arrow")
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED)

# Import (conda-installed-)Boost build configs
find_package(Boost CONFIG COMPONENTS filesystem python REQUIRED)
message(STATUS "Found Boost_DIR: ${Boost_DIR}")

# Build fitCppSDK library
file(GLOB SDKSRC "${FITSDK_VERSION}/cpp/*.cpp")
add_library(fitsdk SHARED ${SDKSRC})
target_include_directories(fitsdk PUBLIC "${FITSDK_VERSION}/cpp")

# Build FitSDK decoder executable
add_executable(decoder "${FITSDK_VERSION}/cpp/examples/decode.cpp")
target_link_libraries(decoder PRIVATE fitsdk)

# Build fittransformer executable 
add_executable(fittransformer fittransformer.cc)
target_link_libraries(fittransformer PRIVATE arrow_shared parquet_shared Boost::filesystem fitsdk)

# Build the fittransformer_so module (uses pybind11)
set(PYBIND11_PYTHON_VERSION ${Boost_PYTHON_VERSION})
set(pybind11_DIR "${CMAKE_INSTALL_PREFIX}/share/cmake/pybind11")
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(fittransformer_so fittransformer.cc fittransformer_so.cc)
target_link_libraries(fittransformer_so PRIVATE arrow_shared parquet_shared
    fitsdk Boost::filesystem pybind11::module pybind11::lto)
