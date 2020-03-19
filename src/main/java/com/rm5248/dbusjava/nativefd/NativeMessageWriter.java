package com.rm5248.dbusjava.nativefd;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import jnr.posix.POSIXFactory;
import org.freedesktop.dbus.FileDescriptor;
import org.freedesktop.dbus.messages.Message;
import org.freedesktop.dbus.spi.IMessageWriter;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author robert
 */
public class NativeMessageWriter implements IMessageWriter {

    private static jnr.posix.POSIX POSIX = POSIXFactory.getPOSIX();
    private final Logger logger = LoggerFactory.getLogger(getClass());
    private static final Logger logger_native = LoggerFactory.getLogger( NativeMessageReader.class.getName() + ".native" );

    private int m_fd;
    private boolean m_isClosed;
    private int m_nativeHandle;

    public NativeMessageWriter( int fd ){
        m_fd = fd;
        m_isClosed = false;
        m_nativeHandle = openNativeHandle( m_fd );
    }

    @Override
    public void writeMessage(Message m) throws IOException {
        ByteArrayOutputStream bos = new ByteArrayOutputStream( 1024 );
        int[] fds = new int[ m.getFiledescriptors().size() ];
        int loc = 0;

        logger.debug("<= {}", m);
        if (null == m) {
            return;
        }
        if (null == m.getWireData()) {
            logger.warn("Message {} wire-data was null!", m);
            return;
        }
        
        for (byte[] buf : m.getWireData()) {
            if( buf == null ){
                break;
            }
            bos.write(buf);
        }

        for( FileDescriptor fd : m.getFiledescriptors() ){
            fds[ loc++ ] = fd.getIntFileDescriptor();
        }
        
        writeNative( m_nativeHandle, bos.toByteArray(), fds );
    }

    @Override
    public boolean isClosed() {
        return m_isClosed;
    }

    @Override
    public void close() throws IOException {
        if( m_isClosed ) return;
        m_isClosed = true;
        POSIX.close( m_fd );
        closeNativeHandle( m_nativeHandle );
    }

    /**
     * Given a filedescriptor, return a native handle to native data.
     * @param fd
     * @return
     */
    private native int openNativeHandle( int fd );

    private native void closeNativeHandle( int handle );

    private native void writeNative( int handle, byte[] msgdata, int[] filedescriptors ) throws IOException;

}
