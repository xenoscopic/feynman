#Set the minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

#Create the project
PROJECT(FeynmanCIntegrationExample)

#Add some custom CMake extensions
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

#Find GSL
FIND_PACKAGE(GSL REQUIRED)

#Add include directories
INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIRS})

#Add main executable
ADD_EXECUTABLE(integrate main.c)

#Set up linking
TARGET_LINK_LIBRARIES(integrate 
                      ${GSL_LIBRARIES})