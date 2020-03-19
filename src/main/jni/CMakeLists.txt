PROJECT( dbus-java-native )

CMAKE_MINIMUM_REQUIRED( VERSION 3.0 )

find_package( JNI )

if ( NOT JNI_FOUND)
    message( FATAL_ERROR "No JNI found, can't compile" )
else()
    message (STATUS "JNI_INCLUDE_DIRS=${JNI_INCLUDE_DIRS}")
    message (STATUS "JNI_LIBRARIES=${JNI_LIBRARIES}")
endif()

INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})
INCLUDE_DIRECTORIES( ../../../target/headers/ )

ADD_LIBRARY( dbus-java-jni-connector SHARED 
	native-message-reader.c 
	native-message-writer.c 
	jni_utils.c )
