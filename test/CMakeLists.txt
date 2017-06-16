
set(SOURCE_FILES)
list(APPEND SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/dag_slicer.cpp)
list(APPEND SOURCE_FILES ${CMAKE_SOURCE_DIR}/src/slicer.cpp)

add_executable(test_slicer test_slicer.cpp ${SOURCE_FILES})

message(STATUS ${MOAB_INCLUDES})
message(STATUS ${MOAB_LINK_LIBS})

include_directories(${CMAKE_SOURCE_DIR}/src/)
include_directories(${MOAB_INCLUDE_DIRS})

target_link_libraries(test_slicer ${MOAB_LIBRARIES})

EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/cube.h5m ${CMAKE_CURRENT_BINARY_DIR}/cube.h5m)
EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/teapot_grps_zip.h5m ${CMAKE_CURRENT_BINARY_DIR}/teapot_grps_zip.h5m)

add_test(NAME test_slicer COMMAND test_slicer WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test)
