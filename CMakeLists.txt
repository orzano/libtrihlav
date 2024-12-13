cmake_minimum_required( VERSION 3.7.2 )

set( APPLICATION_NAME libtrihlav )
set( APPLICATION_VERSION "24.12.01" )

project( libtrihlav VERSION ${APPLICATION_VERSION} )

add_definitions( -DLIB_TRH_NAME="${APPLICATION_NAME}" )
add_definitions( -DLIB_TRH_VERSION="${APPLICATION_VERSION}" )
add_definitions( -DLIB_TRH_VERSION_MAJOR=${CMAKE_PROJECT_VERSION_MAJOR} )
add_definitions( -DLIB_TRH_VERSION_MINOR=${CMAKE_PROJECT_VERSION_MINOR} )
add_definitions( -DLIB_TRH_VERSION_PATCH=${CMAKE_PROJECT_VERSION_PATCH} )

set( CMAKE_C_STANDARD 11 )
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

include_directories( include )

file( GLOB SOURCE_FILES src/*.c )

add_library( libtrihlav SHARED ${SOURCE_FILES} )

target_link_libraries(
	libtrihlav
	json-c
	mnl
)

set_property( TARGET libtrihlav PROPERTY C_STANDARD 11 )
set_target_properties( libtrihlav
  PROPERTIES
  VERSION ${APPLICATION_VERSION}
  SOVERSION 1
)

install( TARGETS libtrihlav LIBRARY DESTINATION /usr/lib/ )
