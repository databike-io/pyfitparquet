cmake_minimum_required(VERSION 3.15)

# =========
# Configure
# =========

# Set project name and lang: C++
project(pyfitparquet LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Don't strip RPATH on install
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# FitSDK version
set(FITSDK_VERSION FitCppSDK_21.47.00)

# Import Apache, Boost, Python, pybind11 build configs
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED HINTS ${Arrow_DIR})
find_package(Boost CONFIG COMPONENTS filesystem REQUIRED)
find_package(Python COMPONENTS Interpreter Development)
find_package(pybind11 CONFIG REQUIRED) # Uses found Python version

message(STATUS "Found Arrow_DIR: ${Arrow_DIR}")
message(STATUS "Found Parquet_DIR: ${Parquet_DIR}")
message(STATUS "Found Boost_DIR: ${Boost_DIR}")
message(STATUS "Found Python_SITELIB: ${Python_SITELIB}")
message(STATUS "Found Python_VERSION: ${Python_VERSION}")

# =====
# Build
# =====

# Build fitCppSDK library
file(GLOB SDKSRC "../${FITSDK_VERSION}/cpp/*.cpp")
add_library(fitsdk SHARED ${SDKSRC})
target_include_directories(fitsdk PUBLIC "../${FITSDK_VERSION}/cpp")

# Build FitSDK decoder executable
add_executable(fitdecoder "../${FITSDK_VERSION}/cpp/examples/decode.cpp")
target_link_libraries(fitdecoder PRIVATE fitsdk)

# Build fittransformer executable 
add_executable(fittransformer fittransformer.cc)
target_link_libraries(fittransformer PRIVATE arrow_shared parquet_shared 
    Boost::filesystem fitsdk)

# Build fittransformer_so cpython module
pybind11_add_module(fittransformer_so fittransformer.cc fittransformer_so.cc)
target_compile_definitions(fittransformer_so PRIVATE -DPYBIND11_PRINT_PYSTDOUT)
target_link_libraries(fittransformer_so PRIVATE arrow_shared parquet_shared
    Boost::filesystem pybind11::module pybind11::lto fitsdk)

# =====================
# Install (for setup.py)
# =====================

# Install py/config files
file(GLOB PYFIT_FILES_PY "../*.py")
file(GLOB PYFIT_FILES_YML "../*.yml")
install(FILES ${PYFIT_FILES_PY} DESTINATION ${INSTALL_SITE_PKGS})
install(FILES ${PYFIT_FILES_YML} DESTINATION ${INSTALL_SITE_PKGS})

# Install lib/bin/so files
install(TARGETS fitsdk LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS fitdecoder RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS fittransformer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS fittransformer_so DESTINATION ${INSTALL_SITE_PKGS})
