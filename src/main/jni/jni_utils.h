/*
 *  Copyright 2020 Robert Middleton <robert.middleton@rm5248.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

/**
 * Utility functions for interacting with the JVM from JNI.
 *
 * To use, copy the header and .c file into whereever you create your JNI code,
 * and add them to your build system to create the .dll/.so
 *
 * Find the latest version at https://github.com/rm5248/jni-utils
 */

#ifndef JNI_UTILS_H
#define JNI_UTILS_H

#include <jni.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A mapping of java.util.logging.Level to C enum
 */
enum JUL_LogLevel {
	JUL_SEVERE,
	JUL_WARNING,
	JUL_INFO,
	JUL_CONFIG,
	JUL_FINE,
	JUL_FINER,
	JUL_FINEST
};

/**
 * A mapping of the different levels of SLF4J to C enum
 */
enum SLF4J_LogLevel {
	SLF4J_TRACE,
	SLF4J_DEBUG,
	SLF4J_INFO,
	SLF4J_WARN,
	SLF4J_ERROR
};

#define JAVA_IO_IOEXCEPTION "java/io/IOException"
#ifdef _WIN32
#define LAST_ERROR GetLastError()
#else
#define LAST_ERROR errno
#endif

/**
 * Convenience macro to throw an IOException with the given message
 */
#define jniutil_throw_ioexception(env, message) jniutil_throw_exception( env, JAVA_IO_IOEXCEPTION, message )

/**
 * Convenince macro to throw an IOException, using the last error(errno on Unix, GetLastError() on Windows)
 */
#define jniutil_throw_ioexception_errnum(env) jniutil_throw_exception_errnum( env, JAVA_IO_IOEXCEPTION, LAST_ERROR )

/**
 * Throw an exception of the given class with the given message
 *
 * @param env The JNI Environment as passed by the JVM
 * @param exception_class_name The name of the Exception class to throw, e.g. java/io/IOException
 * @param message The message for the exception
 */
void jniutil_throw_exception( JNIEnv* env, const char* exception_class_name, const char* message );

/**
 * Throw an exception, using the specified errnum to generate the message
 *
 * @param env The JNI Environment as passed by the JVM
 * @param exception_class_name The name of the Exception class to throw, e.g. java/io/IOException
 * @param errnum The last error number, e.g. either errno or GetLastError()(see also LAST_ERROR macro)
 */
void jniutil_throw_exception_errnum( JNIEnv* env, const char* exception_class_name, int errnum );

/**
 * Log a message using java.util.Logger
 *
 * To use this method correctly, you must have a static instance of java.util.Logger inside of {@class_name},
 * with the name of {@logger_name}.
 *
 * Example:
 * package foo;
 * class FooClass {
 *     private static final java.util.logging.Logger native_logger = java.util.logging.Logger.getLogger( FooClass.class.getName() + ".native" );
 * }
 *
 * JNI:
 * jniutil_jul_log( env, "foo/FooClass", "native_logger", JUL_INFO, "Info message" );
 *
 * @param env The JNI Environment as passed by the JVM
 * @param class_name The class where the logger exists
 * @param logger_name The name of the logger within the class
 * @param level The level of the log message to create
 * @param message The message to output to the logger
 */
void jniutil_jul_log_simple( JNIEnv* env, const char* class_name, const char* logger_name, enum JUL_LogLevel level, const char* message );

/**
 * Log a message using java.util.Logger
 *
 * To use this method correctly, you must have a static instance of java.util.Logger inside of {@class_name},
 * with the name of {@logger_name}.
 *
 * Example:
 * package foo;
 * class FooClass {
 *     private static final java.util.logging.Logger native_logger = java.util.logging.Logger.getLogger( FooClass.class.getName() + ".native" );
 * }
 *
 * JNI:
 * jniutil_jul_log( env, "foo/FooClass", "native_logger", JUL_INFO, "Info message" );
 *
 * @param env The JNI Environment as passed by the JVM
 * @param class_name The class where the logger exists
 * @param logger_name The name of the logger within the class
 * @param level The level of the log message to create
 * @param format The format of the message
 * @param ... Arguments for the formatting(snprintf-compatible)
 */
void jniutil_jul_log( JNIEnv* env, const char* class_name, const char* logger_name, enum JUL_LogLevel level, const char* format, ... );

/**
 * Log a message using org.slf4j.Logger
 *
 * To use this method correctly, you must have a static instance of org.slf4j.Logger inside of {@class_name},
 * with the name of {@logger_name}.
 *
 * Example:
 * package foo;
 * class FooClass {
 *     private static final org.slf4j.Logger native_logger = org.slf4j.LoggerFactory.getLogger( FooClass.class.getName() + ".native" );
 * }
 *
 * JNI:
 * jniutil_slf4j_log( env, "foo/FooClass", "native_logger", SLF4J_INFO, "Info message" );
 *
 * @param env The JNI Environment as passed by the JVM
 * @param class_name The class where the logger exists
 * @param logger_name The name of the logger within the class
 * @param level The level of the log message to create
 * @param message The message to output to the logger
 */
void jniutil_slf4j_log_simple( JNIEnv* env, const char* class_name, const char* logger_name, enum SLF4J_LogLevel level, const char* message );

/**
 * Log a message using org.slf4j.Logger
 *
 * To use this method correctly, you must have a static instance of org.slf4j.Logger inside of {@class_name},
 * with the name of {@logger_name}.
 *
 * Example:
 * package foo;
 * class FooClass {
 *     private static final org.slf4j.Logger native_logger = org.slf4j.LoggerFactory.getLogger( FooClass.class.getName() + ".native" );
 * }
 *
 * JNI:
 * jniutil_slf4j_log( env, "foo/FooClass", "native_logger", SLF4J_INFO, "Info message" );
 *
 * @param env The JNI Environment as passed by the JVM
 * @param class_name The class where the logger exists
 * @param logger_name The name of the logger within the class
 * @param level The level of the log message to create
 * @param format The format for the message(snprintf)
 * @param ... Arguments for the formatting(snprintf-compatible)
 */
void jniutil_slf4j_log( JNIEnv* env, const char* class_name, const char* logger_name, enum SLF4J_LogLevel level, const char* format, ... );

/**
 * Get an integer field from the given object.
 *
 * @param env The JNI environment as passed by the JVM
 * @param obj The object to get the field from
 * @param field_name The name of the integer field to get the value of
 * @param value The value of the integer field
 * @return True if we were able to get the field, false otherwise
 */
int jniutil_get_integer_field( JNIEnv* env, jobject obj, const char* field_name, jint* value );

/**
 * Get a boolean field from the given object.
 *
 * @param env The JNI environment as passed by the JVM
 * @param obj The object to get the field from
 * @param field_name The name of the boolean field to get the value of
 * @param value The value of the boolean field
 * @return True if we were able to get the field, false otherwise
 */
int jniutil_get_boolean_field( JNIEnv* env, jobject obj, const char* field_name, jboolean* value );

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
