#include <stdio.h>
#include <string.h>

#include "jni_utils.h"

static void jni_error( const char* format, ... ){
	va_list myargs;
	va_start( myargs, format );

	fflush( stdout );
	fflush( stderr );
	fprintf( stderr, "JNI Error: " );
	vfprintf( stderr, format, myargs );
	fprintf( stderr, "\n" );
	fflush( stdout );
	fflush( stderr );

	va_end( myargs );
}

void jniutil_throw_exception( JNIEnv* env, const char* exception_class_name, const char* message ){
	jclass exception_class;
	(*env)->ExceptionDescribe( env );
	(*env)->ExceptionClear( env );
	exception_class = (*env)->FindClass(env, exception_class_name);
	(*env)->ThrowNew(env, exception_class, message );
}

void jniutil_throw_exception_errnum( JNIEnv* env, const char* exception_class_name, int errnum ){
#ifdef _WIN32
	LPTSTR error_text = NULL;
	jclass exception_class;
	(*env)->ExceptionDescribe( env );
	(*env)->ExceptionClear( env );
	exception_class = (*env)->FindClass(env, exception_class_name);
	
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNumber,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&error_text,
		0,
		NULL );
	(*env)->ThrowNew(env, exception_class, error_text );
	LocalFree( error_text );
#else
	jclass exception_class;
	(*env)->ExceptionDescribe( env );
	(*env)->ExceptionClear( env );
	exception_class = (*env)->FindClass(env, exception_class_name);
	(*env)->ThrowNew(env, exception_class, strerror( errnum ) );
#endif /* _WIN32 */
}

void jniutil_jul_log_simple( JNIEnv* env, const char* class_name, const char* logger_name, enum JUL_LogLevel level, const char* message ){
	jobject log_obj;
	jclass parent_class;
	jclass logger_class;
	jfieldID logger_id;
	jmethodID log_method_id;
	jstring debug_string;

	logger_class = (*env)->FindClass( env, "java/util/logging/Logger" );
	if( logger_class == NULL ){
		jni_error( "Can't find java logger?" );
		return;
	}

	parent_class = (*env)->FindClass( env, class_name );
	if( parent_class == NULL ){
		jni_error( "Can't find parent class(%s)", class_name );
		return;
	}

	logger_id = (*env)->GetStaticFieldID( env, parent_class, logger_name, "Ljava/util/logging/Logger;" );
	if( logger_id == NULL ){
		jni_error( "Can't find logger with given name(%s)", logger_name );
		return;
	}
	

	log_obj = (*env)->GetStaticObjectField( env, parent_class, logger_id );
	if( log_obj == NULL ){
		jni_error( "Logger is null, can't log" );
		return;
	}

	switch( level ){
	case JUL_SEVERE:
		log_method_id = (*env)->GetMethodID( env, logger_class, "severe", "(Ljava/lang/String;)V" );
		break;
	case JUL_WARNING:
		log_method_id = (*env)->GetMethodID( env, logger_class, "warning", "(Ljava/lang/String;)V" );
		break;
	case JUL_INFO:
		log_method_id = (*env)->GetMethodID( env, logger_class, "info", "(Ljava/lang/String;)V" );
		break;
	case JUL_CONFIG:
		log_method_id = (*env)->GetMethodID( env, logger_class, "config", "(Ljava/lang/String;)V" );
		break;
	case JUL_FINE:
		log_method_id = (*env)->GetMethodID( env, logger_class, "fine", "(Ljava/lang/String;)V" );
		break;
	case JUL_FINER:
		log_method_id = (*env)->GetMethodID( env, logger_class, "finer", "(Ljava/lang/String;)V" );
		break;
	case JUL_FINEST:
		log_method_id = (*env)->GetMethodID( env, logger_class, "finest", "(Ljava/lang/String;)V" );
		break;
	}

	if( log_method_id == NULL ){
		jni_error( "Can't find correct method on java.util.logging.Logger" );
		return;
	}

	debug_string = (*env)->NewStringUTF( env, message );
	if( debug_string == NULL ){
		jni_error( "Can't constuct Java string from given string %s", message );
		return;
	}

	(*env)->CallVoidMethod( env, log_obj, log_method_id, debug_string );
}

void jniutil_jul_log( JNIEnv* env, const char* class_name, const char* logger_name, enum JUL_LogLevel level, const char* format, ... ){
	char buffer[ 2048 ];
	va_list myargs;
	va_start( myargs, format );

	vsnprintf( buffer, 2048, format, myargs );

	va_end( myargs );

	jniutil_jul_log_simple( env, class_name, logger_name, level, buffer );
}

void jniutil_slf4j_log_simple( JNIEnv* env, const char* class_name, const char* logger_name, enum SLF4J_LogLevel level, const char* message ){
	jobject log_obj;
	jclass parent_class;
	jclass logger_class;
	jfieldID logger_id;
	jmethodID log_method_id;
	jstring debug_string;

	logger_class = (*env)->FindClass( env, "org/slf4j/Logger" );
	if( logger_class == NULL ){
		jni_error( "Can't find SLF4J logger?" );
		return;
	}

	parent_class = (*env)->FindClass( env, class_name );
	if( parent_class == NULL ){
		jni_error( "Can't find parent class(%s)", class_name );
		return;
	}

	logger_id = (*env)->GetStaticFieldID( env, parent_class, logger_name, "Lorg/slf4j/Logger;" );
	if( logger_id == NULL ){
		jni_error( "Can't find logger with given name(%s)", logger_name );
		return;
	}
	

	log_obj = (*env)->GetStaticObjectField( env, parent_class, logger_id );
	if( log_obj == NULL ){
		jni_error( "Logger is null, can't log" );
		return;
	}

	switch( level ){
	case SLF4J_TRACE:
		log_method_id = (*env)->GetMethodID( env, logger_class, "trace", "(Ljava/lang/String;)V" );
		break;
	case SLF4J_DEBUG:
		log_method_id = (*env)->GetMethodID( env, logger_class, "debug", "(Ljava/lang/String;)V" );
		break;
	case SLF4J_INFO:
		log_method_id = (*env)->GetMethodID( env, logger_class, "info", "(Ljava/lang/String;)V" );
		break;
	case SLF4J_WARN:
		log_method_id = (*env)->GetMethodID( env, logger_class, "warn", "(Ljava/lang/String;)V" );
		break;
	case SLF4J_ERROR:
		log_method_id = (*env)->GetMethodID( env, logger_class, "error", "(Ljava/lang/String;)V" );
		break;
	}

	if( log_method_id == NULL ){
		jni_error( "Can't find correct method on org.slf4j.Logger" );
		return;
	}

	debug_string = (*env)->NewStringUTF( env, message );
	if( debug_string == NULL ){
		jni_error( "Can't constuct Java string from given string %s", message );
		return;
	}

	(*env)->CallVoidMethod( env, log_obj, log_method_id, debug_string );
}

void jniutil_slf4j_log( JNIEnv* env, const char* class_name, const char* logger_name, enum SLF4J_LogLevel level, const char* format, ... ){
	char buffer[ 2048 ];
	va_list myargs;
	va_start( myargs, format );

	vsnprintf( buffer, 2048, format, myargs );

	va_end( myargs );

	jniutil_slf4j_log_simple( env, class_name, logger_name, level, buffer );
}

int jniutil_get_integer_field( JNIEnv* env, jobject obj, const char* field_name, jint* value ){
	jfieldID fid;
	jclass cls = (*env)->GetObjectClass( env, obj );

	fid = (*env)->GetFieldID( env, cls, field_name, "I" );
	if( fid == 0 ){
		jni_error( "Unable to find field %s", field_name );
		return 0;
	}

	*value = (*env)->GetIntField( env, obj, fid );

	return 1;
}

int jniutil_get_boolean_field( JNIEnv* env, jobject obj, const char* field_name, jboolean* value ){
	jfieldID fid;
	jclass cls = (*env)->GetObjectClass( env, obj );

	fid = (*env)->GetFieldID( env, cls, field_name, "Z" );
	if( fid == 0 ){
		jni_error( "Unable to find field %s", field_name );
		return 0;
	}

	*value = (*env)->GetBooleanField( env, obj, fid );

	return 1;
}
