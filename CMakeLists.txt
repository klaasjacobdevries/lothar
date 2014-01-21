# just to surpress a cmake warning, probably lower versions are fine too
cmake_minimum_required(VERSION 2.6)

# dependency checking
INCLUDE(CheckCXXSourceCompiles)
INCLUDE(FindPythonInterp)

# check usb backend
find_path(LIBUSB_INCLUDE libusb-1.0/libusb.h)
find_library(LIBUSB_LIBRARY usb-1.0)

if(LIBUSB_LIBRARY)
  message("Using USB backend libusb-1.0.")
  set(HAVE_LIBUSB 1)
  list(APPEND EXTERNAL_LIBRARIES ${LIBUSB_LIBRARY})
endif()

# win32 may have libusb as libusbx
if(NOT HAVE_LIBUSB)
  find_path(LIBUSBX_INCLUDE libusbx-1.0/libusb.h)
  find_library(LIBUSBX_LIBRARY libusb-1.0)
  if(LIBUSBX_LIBRARY)
    message("Using USB backend libusbx-1.0.")
    set(HAVE_LIBUSBX 1)
    list(APPEND EXTERNAL_LIBRARIES ${LIBUSBX_LIBRARY})
  endif()
endif()

if(NOT HAVE_LIBUSB AND (NOT HAVE_LIBUSBX))
  find_path(LIBUSB_0_1_INCLUDE usb.h)
  find_library(LIBUSB_0_1_LIBRARY usb)
  if(LIBUSB_0_1_LIBRARY)
    message("Using USB backend libusb-0.1.")
    set(HAVE_LIBUSB_0_1 1)
    list(APPEND EXTERNAL_LIBRARIES ${LIBUSB_0_1_LIBRARY})
  else()
    message(FATAL_ERROR "No viable usb-backend found.")
  endif()
endif()

# check bluez (linux bluetooth)
find_path(BLUEZ_INCLUDE bluetooth/bluetooth.h)
find_library(BLUEZ_LIBRARY bluetooth)

if(BLUEZ_LIBRARY AND BLUEZ_INCLUDE)
  message("Using Bluez bluetooth backend.")
  set(HAVE_BLUEZ 1)
  list(APPEND EXTERNAL_LIBRARIES ${BLUEZ_LIBRARY})
endif()

# check windows (bluetooth as part of the operating system, assumed always present
if(WIN32)
  set(HAVE_WIN32_BLUETOOTH 1)
  list(APPEND EXTERNAL_LIBRARIES ws2_32)
endif()

# can we create test code
find_package(GTest) 
if(GTEST_FOUND)
  # check if google mock can be found
  find_path(GMOCK_INCLUDE_DIR gmock/gmock.h)
  find_library(GMOCK_LIBRARIES gmock)

  if(NOT GMOCK_INCLUDE_DIR)
    message(WARNING "Found Google Test, but not Google Mock, you may run into problems compiling the tests (${GTEST_INCLUDE_DIRS})")
  endif()

  set(HAVE_GTEST 1)
endif()

# does the c++-wrapper need TR1 for a shared_ptr?
CHECK_CXX_SOURCE_COMPILES("
#include <memory>

extern std::shared_ptr<int> some_ptr;
int main(){} " HAVE_SHARED_PTR)

# python module
find_package(PythonInterp)

if(PYTHONINTERP_FOUND)
   add_custom_target(pylothar ALL ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/setup.py build COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/setup.py install --prefix=${PROJECT_BINARY_DIR})
   install(DIRECTORY ${PROJECT_BINARY_DIR}/lib/ DESTINATION lib/)
endif()

# doxygen
find_package(Doxygen)

if(DOXYGEN_FOUND)
  add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doxyfile-c COMMAND ${DOXYGEN_EXECUTABLE} ${PROJECT_SOURCE_DIR}/doxyfile-c++ WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

# public headers/include path
set(PUBLIC_HEADER_DIR ${PROJECT_SOURCE_DIR}/src/include)
CONFIGURE_FILE(${PUBLIC_HEADER_DIR}/config.h.in ${PROJECT_BINARY_DIR}/include/config.h)
include_directories(${PUBLIC_HEADER_DIR} ${PROJECT_BINARY_DIR}/include)

# sources
FILE(GLOB PUBLIC_HEADERS ${PUBLIC_HEADER_DIR}/*.h)
FILE(GLOB SOURCES src/*.c src/*.h)
list(APPEND PUBLIC_HEADERS ${PROJECT_BINARY_DIR}/include/config.h)

# usb backend
if(HAVE_LIBUSB OR HAVE_LIBUSBX)
  list(APPEND SOURCES src/usb/libusb.c)
elseif(HAVE_LIBUSB_0_1)
  list(APPEND SOURCES src/usb/libusb-0.1.c)
else(HAVE_LIBUSB)
  list(APPEND SOURCES src/usb/dummyusb.c)
endif(HAVE_LIBUSB OR HAVE_LIBUSBX)

# bluetooth backend
if(HAVE_BLUEZ)
  LIST(APPEND SOURCES src/bluetooth/bluez.c)
elseif(HAVE_WIN32_BLUETOOTH)
  LIST(APPEND SOURCES src/bluetooth/win32bluetooth.c)
else(HAVE_BLUEZ)
  LIST(APPEND SOURCES src/bluetooth/dummybluetooth.c)
endif(HAVE_BLUEZ)

# library
add_library(lothar STATIC ${SOURCES} ${PUBLIC_HEADERS})
target_link_libraries(lothar ${EXTERNAL_LIBRARIES})

# c++ backend
# sources
set(CC_PUBLIC_HEADER_DIR ${PROJECT_SOURCE_DIR}/wrappers/c++/include)
include_directories(${CC_PUBLIC_HEADER_DIR})
FILE(GLOB CC_PUBLIC_HEADERS ${CC_PUBLIC_HEADER_DIR}/*.hh)
FILE(GLOB CC_SOURCES wrappers/c++/*.cc wrappers/c++/*.hh)

add_library(lothar++ STATIC ${CC_SOURCES} ${CC_PUBLIC_HEADERS})
add_dependencies(lothar++ lothar)
target_link_libraries(lothar++ lothar)

# test code
if(HAVE_GTEST)
  find_package(Threads)
  FILE(GLOB TEST_SOURCES tests/*.cc)
  include_directories(${GTEST_INCLUDE_DIRS})
  add_executable(lothar_tests ${TEST_SOURCES})
  add_dependencies(lothar_tests lothar++ lothar)
  target_link_libraries(lothar_tests lothar++ lothar ${GTEST_BOTH_LIBRARIES} ${GMOCK_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif(HAVE_GTEST)

# install rules
install(TARGETS lothar lothar++ DESTINATION lib)
install(FILES ${PUBLIC_HEADERS} DESTINATION include/lothar)
install(FILES ${CC_PUBLIC_HEADERS} DESTINATION include/lothar)
