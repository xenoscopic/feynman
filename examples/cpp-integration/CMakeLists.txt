#Set the minimum CMake version
CMAKE_MINIMUM_REQUIRED(VERSION 2.6.4)

#Create the project
PROJECT(FeynmanCIntegrationExample)

#Set the path to the Feynman source
SET(FEYNMAN_SOURCE_PATH "${PROJECT_SOURCE_DIR}/../..")

#Add some custom CMake extensions
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${FEYNMAN_SOURCE_PATH}/share/cmake)

#Just a little utility command for wrapping strings
macro(join_arguments var)
  set(_var)
  foreach(_v ${${var}})
    set(_var "${_var} ${_v}")
  endforeach(_v ${${var}})
  string(STRIP ${_var} _var)
  set(${var} ${_var})
endmacro(join_arguments)

#Find GSL
FIND_PACKAGE(GSL)
IF(GSL_FOUND)
    ADD_DEFINITIONS(-DHAVE_GSL)
    INCLUDE_DIRECTORIES(${GSL_INCLUDE_DIRS})
    SET(LINK_LIBRARIES ${LINK_LIBRARIES} ${GSL_LIBRARIES})
ENDIF(GSL_FOUND)

#Find OpenCL
FIND_PACKAGE(OpenCL)
IF(OPENCL_FOUND)
    ADD_DEFINITIONS(-DHAVE_OPENCL)
    INCLUDE_DIRECTORIES(${OPENCL_INCLUDE_DIRS})
    SET(LINK_LIBRARIES ${LINK_LIBRARIES} ${OPENCL_LIBRARIES})
ENDIF(OPENCL_FOUND)

#Set the integrand paths
SET(INTEGRAND_HEADER_PATH ${FEYNMAN_SOURCE_PATH}/share/sample_integrands.h)
SET(INTEGRAND_SOURCE_PATH ${FEYNMAN_SOURCE_PATH}/share/sample_integrands.cpp)

#Set the integrand function names
SET(INTEGRAND_FUNCTIONS
    "unit_cylinder"
    "random_walk"
)

#Create the integrator output directories
SET(HEADER_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/include")
SET(SOURCE_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/src")
FILE(MAKE_DIRECTORY "${HEADER_OUTPUT_DIRECTORY}")
FILE(MAKE_DIRECTORY "${SOURCE_OUTPUT_DIRECTORY}")

#Add to include paths
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${FEYNMAN_SOURCE_PATH}/share)

#Set the integration types
IF(GSL_FOUND)
    SET(INTEGRATION_TYPES
        ${INTEGRATION_TYPES}
        "gsl-plain"
        "gsl-miser"
        "gsl-vegas"
    )
ENDIF(GSL_FOUND)
IF(OPENCL_FOUND)
    SET(INTEGRATION_TYPES
        ${INTEGRATION_TYPES}
        "opencl-plain"
    )
ENDIF(OPENCL_FOUND)

#Create each integration variant
FOREACH(t ${INTEGRATION_TYPES})
    #Compute the file prefix
    STRING(REPLACE "-" "_" FILE_SUFFIX ${t})
    SET(FILE_SUFFIX "_${FILE_SUFFIX}_integrator")

    #Loop over each integrand function
    FOREACH(INTEGRAND ${INTEGRAND_FUNCTIONS})
        #Compute the header/source output path
        SET(HEADER_OUTPUT_PATH "${HEADER_OUTPUT_DIRECTORY}/${INTEGRAND}${FILE_SUFFIX}.h")
        SET(SOURCE_OUTPUT_PATH "${SOURCE_OUTPUT_DIRECTORY}/${INTEGRAND}${FILE_SUFFIX}.cpp")

        #Compute the output function name
        SET(OUTPUT_CLASS_NAME "${INTEGRAND}${FILE_SUFFIX}")

        #Create the command
        ADD_CUSTOM_COMMAND(OUTPUT ${HEADER_OUTPUT_PATH} ${SOURCE_OUTPUT_PATH}
                           COMMAND ${FEYNMAN_SOURCE_PATH}/genint.py --integrand-file ${INTEGRAND_SOURCE_PATH} --integrand-name ${INTEGRAND} --dependencies sample_integrands.h --header-output-path ${HEADER_OUTPUT_PATH} --source-output-path ${SOURCE_OUTPUT_PATH} --integrator-name ${OUTPUT_CLASS_NAME} --backend ${t}
                           VERBATIM)
        
        #Record the target and output files
        SET(INTEGRATOR_HEADERS
            ${INTEGRATOR_HEADERS}
            ${HEADER_OUTPUT_PATH})
        SET(INTEGRATOR_SOURCES
            ${INTEGRATOR_SOURCES}
            ${SOURCE_OUTPUT_PATH})
    ENDFOREACH(INTEGRAND)
ENDFOREACH(t)

#Add the integration target
ADD_CUSTOM_TARGET(INTEGRATOR_CODE_TARGET
                  DEPENDS ${INTEGRATOR_HEADERS} ${INTEGRATOR_SOURCES})

#Add the integrand library
ADD_LIBRARY(integrands SHARED ${INTEGRAND_SOURCE_PATH})

#Add the integrator library
ADD_LIBRARY(integrators SHARED ${INTEGRATOR_SOURCES})
ADD_DEPENDENCIES(integrators INTEGRATOR_CODE_TARGET)

#Add main executable
ADD_EXECUTABLE(integrate main.cpp)

#Set up linking
TARGET_LINK_LIBRARIES(integrators
                      integrands
                      ${LINK_LIBRARIES})
TARGET_LINK_LIBRARIES(integrate 
                      integrators)
