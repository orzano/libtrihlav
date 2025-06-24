cmake_minimum_required( VERSION 3.10 )

set( APPLICATION_NAME trihlav )
set( APPLICATION_VERSION "25.06.24" )

project( trihlav VERSION ${APPLICATION_VERSION} )

add_definitions( -DLIB_TRH_NAME="${APPLICATION_NAME}" )
add_definitions( -DLIB_TRH_VERSION="${APPLICATION_VERSION}" )
add_definitions( -DLIB_TRH_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} )
add_definitions( -DLIB_TRH_VERSION_MINOR=${PROJECT_VERSION_MINOR} )
add_definitions( -DLIB_TRH_VERSION_PATCH=${PROJECT_VERSION_PATCH} )

set( CMAKE_C_STANDARD 11 )
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11" )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )

include_directories( include )

file( GLOB SOURCE_FILES src/*.c )

add_library( trihlav SHARED ${SOURCE_FILES} )

target_link_libraries(
	trihlav
	json-c
	systemd
	mnl
)

set_property( TARGET trihlav PROPERTY C_STANDARD 11 )
set_target_properties( trihlav
  PROPERTIES
  VERSION ${APPLICATION_VERSION}
  SOVERSION 1
)

install( TARGETS trihlav CONFIGURATIONS ${CMAKE_BUILD_TYPE} LIBRARY DESTINATION  "${CMAKE_INSTALL_LIB}" )
