cmake_minimum_required(VERSION 3.15)

# =========
# Configure
# =========

# Set project name and lang: C++
project(pyfitparquet LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# FitSDK version
set(FITSDK_VERSION FitCppSDK_21.47.00)

# Install with relative RPATHs 
if(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib") # MacOS
else()
    set(CMAKE_INSTALL_RPATH "\$ORIGIN/../lib") # Linux/Solaris
endif()

# Import pybind11 build config (relative to Python version)
set(PYTHON_EXECUTABLE "${CMAKE_PREFIX_PATH}/bin/python")
find_package(pybind11 CONFIG REQUIRED) 

# Import Apache, Boost build configs
find_package(Arrow CONFIG REQUIRED)
find_package(Parquet CONFIG REQUIRED HINTS ${Arrow_DIR})
find_package(Boost CONFIG COMPONENTS filesystem REQUIRED)

message(STATUS "Found Arrow_DIR: ${Arrow_DIR}")
message(STATUS "Found Parquet_DIR: ${Parquet_DIR}")
message(STATUS "Found Boost_DIR: ${Boost_DIR}")

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

# ======================
# Install (for setup.py)
# ======================

# Install py/config files
file(GLOB PYFIT_FILES_PY "../*.py")
file(GLOB PYFIT_FILES_YML "../*.yml")
install(FILES ${PYFIT_FILES_PY} DESTINATION ${INSTALL_SITE_PKGS})
install(FILES ${PYFIT_FILES_YML} DESTINATION ${INSTALL_SITE_PKGS})

# Install lib/bin/so files
install(TARGETS fittransformer_so DESTINATION ${INSTALL_SITE_PKGS})
install(TARGETS fitsdk LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(TARGETS fitdecoder RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(TARGETS fittransformer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
