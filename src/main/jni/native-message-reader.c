#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "com_rm5248_dbusjava_nativefd_NativeMessageReader.h"
#include "jni_utils.h"

struct ReceiveHandle {
	struct msghdr msg_data;
	int rx_iovlen;
	int rx_controllen;
	int fd;
};

static struct ReceiveHandle** rx_array = NULL;
static int rx_array_length = 0;

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageReader
 * Method:    openNativeHandle
 * Signature: (I)I
 */
JNIEXPORT jint JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageReader_openNativeHandle
  (JNIEnv * env, jobject obj, jint fd){
	struct ReceiveHandle* new_rx_handle;
	int list_pos;
	int found = 0;

	/* First time - allocate 10 ReceiveHandles */
	if( rx_array == NULL ){
		rx_array = calloc( rx_array_length + 10, sizeof( struct ReceiveHandle* ) );
		rx_array_length = 10;
	}

	/* Look for a free index into our array */
	for( list_pos = 0; list_pos < rx_array_length; list_pos++ ){
		if( rx_array[ list_pos] == NULL ){
			found = 1;
			break;
		}
	}

	if( !found ){
		/* No free slots - expand our array by 10 elements */
		struct ReceiveHandle** new_rx_array = calloc( rx_array_length + 10, sizeof( struct ReceiveHandle* ) );

		for( list_pos = 0; list_pos < rx_array_length; list_pos++ ){
			new_rx_array[ list_pos ] = rx_array[ list_pos ];
		}
		list_pos += 1;
		rx_array_length += 10;

		free( rx_array );
		rx_array = new_rx_array;
	}

	new_rx_handle = malloc( sizeof( struct ReceiveHandle ) );
	memset( new_rx_handle, 0, sizeof( struct ReceiveHandle ) );

	new_rx_handle->rx_iovlen = 1024;
	new_rx_handle->rx_controllen = 512;
	new_rx_handle->fd = fd;

	/* Allocate some place for data, as we need to peek at messages */
	new_rx_handle->msg_data.msg_iov = malloc( sizeof( struct iovec ) );
	new_rx_handle->msg_data.msg_iov[0].iov_base = malloc( new_rx_handle->rx_iovlen );
	new_rx_handle->msg_data.msg_iov[0].iov_len = new_rx_handle->rx_iovlen;
	new_rx_handle->msg_data.msg_iovlen = 1;
	new_rx_handle->msg_data.msg_control = malloc( new_rx_handle->rx_controllen );

	rx_array[ list_pos ] = new_rx_handle;

	return list_pos;
}

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageReader
 * Method:    closeNativeHandle
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageReader_closeNativeHandle
  (JNIEnv * env, jobject obj, jint handle){
	struct ReceiveHandle* rx_handle = rx_array[ handle ];

	free( rx_handle->msg_data.msg_control );
	free( rx_handle->msg_data.msg_iov[0].iov_base );
	free( rx_handle->msg_data.msg_iov );
	free( rx_handle );
	rx_array[ handle ] = NULL;
}

/*
 * Class:     com_rm5248_dbusjava_nativefd_NativeMessageReader
 * Method:    readNative
 * Signature: (I)Lcom/rm5248/dbus/java/nativefd/MsgHdr;
 */
JNIEXPORT jobject JNICALL Java_com_rm5248_dbusjava_nativefd_NativeMessageReader_readNative
  (JNIEnv * env, jobject obj, jint handle){
	struct ReceiveHandle* rx_handle = rx_array[ handle ];
	ssize_t ret;
	ssize_t header_array_len;
	ssize_t body_len;
	ssize_t total_len;
	ssize_t num_fds;
	uint8_t* header_raw = rx_handle->msg_data.msg_iov[0].iov_base;
	jclass msghdr_class;
	jmethodID constructor_id;
	jintArray fd_array = NULL;
	jbyteArray data_array;
	struct cmsghdr* cmsg;

	/* Do a peek of the data to determine if our arrays are large enough or not */
	rx_handle->msg_data.msg_namelen = 0;
	rx_handle->msg_data.msg_iov[0].iov_len = 16;
	rx_handle->msg_data.msg_controllen = 0;

	ret = recvmsg( rx_handle->fd, &rx_handle->msg_data, MSG_PEEK );
	if( ret < 0 ){
		jniutil_throw_ioexception_errnum(env);
		return NULL;
	}

	jniutil_slf4j_log( env,
		"com/rm5248/dbusjava/nativefd/NativeMessageReader",
		"logger_native",
		SLF4J_DEBUG,
		"Received message.  Ret: %d iovlen: %d controllen: %d",
		ret,
		rx_handle->msg_data.msg_iovlen, 
		rx_handle->msg_data.msg_controllen );

	if( header_raw[0] == 'l' ){
		/* Little-endian */
		body_len = header_raw[ 7 ] << 24 |
			header_raw[ 6 ] << 16 |
			header_raw[ 5 ] << 8 |
			header_raw[ 4 ] << 0;

		header_array_len = header_raw[ 15 ] << 24 |
			header_raw[ 14 ] << 16 |
			header_raw[ 13 ] << 8 |
			header_raw[ 12 ] << 0;
	}else if( header_raw[0] == 'B' ){
		/* Big-endian */
		body_len = header_raw[ 4 ] << 24 |
			header_raw[ 5 ] << 16 |
			header_raw[ 6 ] << 8 |
			header_raw[ 7 ] << 0;

		header_array_len = header_raw[ 12 ] << 24 |
			header_raw[ 13 ] << 16 |
			header_raw[ 14 ] << 8 |
			header_raw[ 15 ] << 0;
	}else{
		jniutil_slf4j_log( env,
			"com/rm5248/dbusjava/nativefd/NativeMessageReader",
			"logger_native",
			SLF4J_ERROR,
			"Unknown endianess!" );
		jniutil_throw_ioexception( env, "Unknown endiannes coming from DBus" );
		return NULL;
	}

	if( 0 != header_array_len % 8 ){
		header_array_len += 8 - (header_array_len % 8);
	}
	total_len = 12 + ( 4 + header_array_len ) + body_len;

	/* Check to see if our iovlen is big enough - expand if not */
	if( rx_handle->rx_iovlen < total_len ){
		free( rx_handle->msg_data.msg_iov[0].iov_base );
		rx_handle->msg_data.msg_iov[0].iov_base = malloc( total_len );
		rx_handle->rx_iovlen = total_len;
	}

	/* Set the amount of data that we want to read to the size of the message */
	rx_handle->msg_data.msg_iov[0].iov_len = total_len;
	rx_handle->msg_data.msg_controllen = rx_handle->rx_controllen;

	/* Do the real reading of the data */
	ret = recvmsg( rx_handle->fd, &rx_handle->msg_data, 0 );
	if( ret < 0 ){
		jniutil_throw_ioexception_errnum(env);
		return NULL;
	}

	/* Need to figure out how many FDs we have to create our array */
	for( cmsg = CMSG_FIRSTHDR(&rx_handle->msg_data);
		cmsg != NULL;
		cmsg = CMSG_NXTHDR(&rx_handle->msg_data, cmsg) ) {
		if( cmsg->cmsg_level == SOL_SOCKET &&
			cmsg->cmsg_type == SCM_RIGHTS ){
			/* This is our FD array */
			num_fds = CMSG_LEN( cmsg->cmsg_len ) / sizeof( int );
			fd_array = (*env)->NewIntArray( env, num_fds );
			(*env)->SetIntArrayRegion( env, fd_array, 0, num_fds, CMSG_DATA( cmsg ) );
		}
	}

	/* Create the new Java object */
	msghdr_class = (*env)->FindClass( env, "com/rm5248/dbusjava/nativefd/MsgHdr" );
	constructor_id = (*env)->GetMethodID( env, msghdr_class, "<init>", "([B[I)V" );
	data_array = (*env)->NewByteArray( env, total_len );
	(*env)->SetByteArrayRegion( env, data_array, 0, total_len, rx_handle->msg_data.msg_iov[0].iov_base );
	return (*env)->NewObject( env, msghdr_class, constructor_id, data_array, fd_array );
}

