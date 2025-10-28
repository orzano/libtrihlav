cmake_minimum_required( VERSION 3.10 )

set( APPLICATION_NAME trihlav )
set( APPLICATION_VERSION "25.08.04" )

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

# Add an option to control whether to build a static or shared library
option( TRIHLAV_LINK_STATIC "Build Trihlav as a static library" OFF )

include_directories( include )

file( GLOB SOURCE_FILES src/*.c )

if( TRIHLAV_LINK_STATIC )
	add_library( ${APPLICATION_NAME} STATIC ${SOURCE_FILES} )
	set_target_properties( ${APPLICATION_NAME} PROPERTIES VERSION ${APPLICATION_VERSION} )
	set( TRIHLAV_INSTALL_DEST "ARCHIVE" )
else()
	add_library( ${APPLICATION_NAME} SHARED ${SOURCE_FILES} )
	set_target_properties( ${APPLICATION_NAME} PROPERTIES VERSION ${APPLICATION_VERSION} SOVERSION 1 )
	set( TRIHLAV_INSTALL_DEST "LIBRARY" )
	install( TARGETS ${APPLICATION_NAME} CONFIGURATIONS ${CMAKE_BUILD_TYPE} ${TRIHLAV_INSTALL_DEST} DESTINATION "${CMAKE_INSTALL_LIB}" )
endif()

set_property( TARGET trihlav PROPERTY C_STANDARD 11 )
target_link_libraries(
	${APPLICATION_NAME}
	json-c
	systemd
)

if( CMAKE_BUILD_TYPE STREQUAL "Debug" )
	target_link_options( ${APPLICATION_NAME} PRIVATE -rdynamic )
endif()
