project(DAGMC-Slicer)

cmake_minimum_required(VERSION 2.8)

# Allow use of find_package(HDF5), find_package(MOAB), etc.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR}/cmake)

find_package(MOAB REQUIRED)

set(MOAB_LINK_LIBS "${MOAB_LIBRARIES}")
set(MOAB_INCLUDES "${MOAB_INCLUDE_DIRS}")

add_subdirectory(src)
