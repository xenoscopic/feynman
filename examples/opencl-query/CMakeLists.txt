#Set the minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

#Create the project
PROJECT(FeynmanOpenCLQuery)

#Set the path to the Feynman source
SET(FEYNMAN_SOURCE_PATH "${PROJECT_SOURCE_DIR}/../..")

#Add some custom CMake extensions
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${FEYNMAN_SOURCE_PATH}/share/cmake)

#Find OpenCL
FIND_PACKAGE(OpenCL REQUIRED)
ADD_DEFINITIONS(-DHAVE_OPENCL)
INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIRS})
SET(LINK_LIBRARIES ${LINK_LIBRARIES} ${OPENCL_LIBRARIES})

#Add local includes
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})

#Add main executable
ADD_EXECUTABLE(query main.c iuncl.c)

#Set up linking
TARGET_LINK_LIBRARIES(query
                      ${LINK_LIBRARIES})
