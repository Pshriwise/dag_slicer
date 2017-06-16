

set(SOURCE_FILES)
list(APPEND SOURCE_FILES dag_slicer.cpp)
list(APPEND SOURCE_FILES slicer.cpp)

add_executable(model_slicer model_slicer.cpp ${SOURCE_FILES})

message(STATUS ${MOAB_INCLUDES})
message(STATUS ${MOAB_LINK_LIBS})

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${MOAB_INCLUDE_DIRS})

target_link_libraries(model_slicer ${MOAB_LIBRARIES})

install(TARGETS model_slicer DESTINATION bin)
