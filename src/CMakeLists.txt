

SET(SOURCE_FILES)
LIST(APPEND SOURCE_FILES dag_slicer.cpp)
LIST(APPEND SOURCE_FILES slicer.cpp)

# add target
ADD_EXECUTABLE(model_slicer model_slicer.cpp ${SOURCE_FILES})

# include sources from "src" dir and from MOAB
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR})
INCLUDE_DIRECTORIES(${MOAB_INCLUDE_DIRS})

# link slicer to MOAB libraries
TARGET_LINK_LIBRARIES(model_slicer ${MOAB_LIBRARIES})

# install slicer program
INSTALL(TARGETS model_slicer DESTINATION bin)
