cmake_minimum_required(VERSION 3.16)
project(PlutoSDR C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(SDR_PREFIX "${CMAKE_SOURCE_DIR}/install" CACHE PATH "Prefix with SoapySDR")

find_library(SOAPYSDR_LIB SoapySDR PATHS "${SDR_PREFIX}/lib" NO_DEFAULT_PATH REQUIRED)
find_path(SOAPYSDR_INC SoapySDR/Device.h PATHS "${SDR_PREFIX}/include" NO_DEFAULT_PATH REQUIRED)
message(STATUS "SoapySDR library: ${SOAPYSDR_LIB}")
message(STATUS "SoapySDR include: ${SOAPYSDR_INC}")

add_executable(main src/main.cpp)

target_include_directories(main PRIVATE "${SOAPYSDR_INC}")
target_link_libraries(main PRIVATE "${SOAPYSDR_LIB}")

set_target_properties(main PROPERTIES
    BUILD_RPATH "${SDR_PREFIX}/lib"
    INSTALL_RPATH "${SDR_PREFIX}/lib"
)