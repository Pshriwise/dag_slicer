PROJECT(DAGMC-Slicer)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Allow use of find_package(HDF5), find_package(MOAB), etc.
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

# look for MOAB installation
FIND_PACKAGE(MOAB REQUIRED)

# do work in src and test dirs
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(test)

# add test to build
ENABLE_TESTING()
ADD_TEST(NAME test_slicer COMMAND test_slicer WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)
