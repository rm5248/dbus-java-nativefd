#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "com_rm5248_dbusjava_nativefd_NativeMessageWriter.h"
#include "jni_utils.h"

struct SendHandle {
	struct msghdr msg_data;
	struct iovec msg_iodata;
	int tx_iovlen;
	int tx_fdlen;
	int fd;
	int* fd_array;
	uint8_t* msg_raw;
};

static struct SendHandle** tx_array = NULL;
static int tx_array_length = 0;

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageWriter
 * Method:    openNativeHandle
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageWriter_openNativeHandle
  (JNIEnv * env, jobject obj, jint fd){
	struct SendHandle* new_tx_handle;
	int list_pos;
	int found = 0;

	/* First time - allocate 10 SendHandles */
	if( tx_array == NULL ){
		tx_array = calloc( tx_array_length + 10, sizeof( struct SendHandle* ) );
		tx_array_length = 10;
	}

	/* Look for a free index into our array */
	for( list_pos = 0; list_pos < tx_array_length; list_pos++ ){
		if( tx_array[ list_pos] == NULL ){
			found = 1;
			break;
		}
	}

	if( !found ){
		/* No free slots - expand our array by 10 elements */
		struct SendHandle** new_tx_array = calloc( tx_array_length + 10, sizeof( struct SendHandle* ) );

		for( list_pos = 0; list_pos < tx_array_length; list_pos++ ){
			new_tx_array[ list_pos ] = tx_array[ list_pos ];
		}
		list_pos += 1;
		tx_array_length += 10;

		free( tx_array );
		tx_array = new_tx_array;
	}

	new_tx_handle = malloc( sizeof( struct SendHandle ) );
	memset( new_tx_handle, 0, sizeof( struct SendHandle ) );

	new_tx_handle->fd = fd;
	new_tx_handle->msg_data.msg_iov = &new_tx_handle->msg_iodata;
	new_tx_handle->msg_data.msg_iovlen = 1;

	tx_array[ list_pos ] = new_tx_handle;

	return list_pos;
}

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageWriter
 * Method:    closeNativeHandle
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageWriter_closeNativeHandle
  (JNIEnv * env, jobject obj, jint handle){
	struct SendHandle* tx_handle = tx_array[ handle ];

	free( tx_handle->fd_array );
	free( tx_handle->msg_raw );
	free( tx_handle );
	tx_array[ handle ] = NULL;
}

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageWriter
 * Method:    writeNative
 * Signature: (I[B[I)V
 */
JNIEXPORT void JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageWriter_writeNative
  (JNIEnv * env, jobject obj, jint handle, jbyteArray bytedata, jintArray filedescriptors){
	struct SendHandle* tx_handle = tx_array[ handle ];
	int message_size = (*env)->GetArrayLength( env, bytedata );
	int fds_size = (*env)->GetArrayLength( env, filedescriptors );
	struct cmsghdr* cmsg;
	int fd_space_needed = CMSG_SPACE( sizeof( int ) * fds_size );

	/* Make sure our data buffer is big enough and set the location */
	if( tx_handle->tx_iovlen < message_size ){
		free( tx_handle->msg_raw );
		tx_handle->msg_raw = malloc( message_size );
	}
	tx_handle->msg_iodata.iov_base = tx_handle->msg_raw;
	tx_handle->msg_iodata.iov_len = message_size;

	/* Make sure our FD array is large enough */
	if( tx_handle->tx_fdlen < fd_space_needed ){
		free( tx_handle->fd_array );
		tx_handle->fd_array = malloc( fd_space_needed );
	}

	/* Fill in our FD array(ancillary data) */
	if( fds_size > 0 ){
		tx_handle->msg_data.msg_control = tx_handle->fd_array;
		tx_handle->msg_data.msg_controllen = fd_space_needed;
		cmsg = CMSG_FIRSTHDR( &tx_handle->msg_data );
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;
		cmsg->cmsg_len = CMSG_LEN( sizeof( int ) * fds_size );
		(*env)->GetIntArrayRegion( env, filedescriptors, 0, fds_size, (int*)CMSG_DATA( cmsg ) );
	}

	/* Fill in our data array */
	(*env)->GetByteArrayRegion( env, bytedata, 0, message_size, tx_handle->msg_raw );

	jniutil_slf4j_log( env,
		"com/rm5248/dbusjava/nativefd/NativeMessageWriter",
		"logger_native",
		SLF4J_DEBUG,
		"sending data len %d num FDs %d",
		message_size,
		fds_size );

	/* Now we finally send the data! */
	if( sendmsg( tx_handle->fd, &tx_handle->msg_data, 0 ) < 0 ){
		jniutil_throw_ioexception_errnum(env);
	}
}
