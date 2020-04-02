cmake_minimum_required(VERSION 3.5)
project(eoepcaows)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH OFF)
set(CMAKE_CXX_FLAGS "-fPIC")

set(RUNNER_NAME
		runner
)

#XML dependences--------------------------------------
find_package(LibXml2 REQUIRED)
find_package(LibXslt REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})
include_directories(${LIBXSLT_INCLUDE_DIR})
#--------------------------------------------------------
include_directories(includes)

SET(MODS_LIBS
		z m dl
		)

SET(INCLUDE_SRC
		includes/eoepca/IMod.hpp
		includes/eoepca/owl/eoepcaows.hpp includes/eoepca/owl/owsparameter.hpp)

SET(LIB_INCLUDE

)

SET(LIB_SRC
		lib/eoepcaows.cpp
		lib/parser.cpp lib/includes/parser.hpp includes/eoepca/owl/eoepcaows.hpp)

SET(MAIN_SRC 
	src/run.cpp
)


add_library(${CMAKE_PROJECT_NAME} SHARED ${LIB_SRC}  )
TARGET_LINK_LIBRARIES(${CMAKE_PROJECT_NAME} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${MODS_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

add_executable(${RUNNER_NAME} ${INCLUDE_SRC} ${MAIN_SRC})
TARGET_LINK_LIBRARIES(${RUNNER_NAME} ${MODS_LIBS} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

add_subdirectory(tests)

#/Users/rdirienzo/Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake