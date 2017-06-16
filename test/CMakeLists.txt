
# use source files from "src" dir
SET(SOURCE_FILES)
LIST(APPEND SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/dag_slicer.cpp)
LIST(APPEND SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/slicer.cpp)

# create test_slicer executable
ADD_EXECUTABLE(test_slicer test_slicer.cpp ${SOURCE_FILES})

# include "src" dir and MOAB includes
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/)
INCLUDE_DIRECTORIES(${MOAB_INCLUDE_DIRS})

# link MOAB libraries to the test program
TARGET_LINK_LIBRARIES(test_slicer ${MOAB_LIBRARIES})

# create symlinks of the test mesh files
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/cube.h5m ${CMAKE_CURRENT_BINARY_DIR}/cube.h5m)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/teapot_grps_zip.h5m ${CMAKE_CURRENT_BINARY_DIR}/teapot_grps_zip.h5m)

