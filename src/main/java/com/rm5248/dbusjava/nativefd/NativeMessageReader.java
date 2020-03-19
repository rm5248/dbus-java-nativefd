package com.rm5248.dbusjava.nativefd;

import java.io.EOFException;
import java.io.IOException;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.IntBuffer;
import jnr.posix.POSIXFactory;
import org.freedesktop.dbus.spi.IMessageReader;
import org.freedesktop.dbus.exceptions.DBusException;
import org.freedesktop.dbus.exceptions.MessageProtocolVersionException;
import org.freedesktop.dbus.messages.Message;
import org.freedesktop.dbus.messages.Message.Endian;
import org.freedesktop.dbus.messages.MessageFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Implements a MessageReader that uses JNI code to call `sendmsg`.
 */
public class NativeMessageReader implements IMessageReader {

    private static jnr.posix.POSIX POSIX = POSIXFactory.getPOSIX();
    private final Logger logger = LoggerFactory.getLogger(getClass());
    private static final Logger logger_native = LoggerFactory.getLogger( NativeMessageReader.class.getName() + ".native" );

    private int m_fd;
    private boolean m_isClosed;
    private int m_nativeHandle;

    public NativeMessageReader( int fd ){
        m_fd = fd;
        m_isClosed = false;
        m_nativeHandle = openNativeHandle( m_fd );
    }

    @Override
    public boolean isClosed() {
        return m_isClosed;
    }

    @Override
    public Message readMessage() throws IOException, DBusException {
        int messageBodyLen;
        int headerArrayLen;
        int totalMessageLen;
        MsgHdr h = readNative( m_nativeHandle );
        logger.debug( "Got the data" );
        ByteBuffer inData = ByteBuffer.wrap( h.getMessages().get( 0 ) );

        /* Parse the details from the header */
        byte endian = inData.get(0);
        byte type = inData.get(1);
        byte protover = inData.get(3);
        if (protover > Message.PROTOCOL) {
            throw new MessageProtocolVersionException(String.format("Protocol version %s is unsupported", protover));
        }

        if( endian == Endian.BIG ){
            inData.order(ByteOrder.BIG_ENDIAN);
        }else if( endian == Endian.LITTLE ){
            inData.order(ByteOrder.LITTLE_ENDIAN);
        }else{
            logger.error( "Incorrect endian {}", endian );
            throw new IOException( "incorrect endianess" );
        }

        IntBuffer asInt = inData.asIntBuffer();

        //Get the length of the body
        messageBodyLen = asInt.get(1);

        // Get the length of the header array
        headerArrayLen = asInt.get(3);
        if (0 != headerArrayLen % 8) {
            headerArrayLen += 8 - (headerArrayLen % 8);
        }

        totalMessageLen = 12 + (4 + headerArrayLen) + messageBodyLen;
        
        if( totalMessageLen != h.getMessages().get( 0 ).length ){
            throw new IOException( "Incorrect length of data read from JNI" );
        }

        byte[] header1 = new byte[12];
        byte[] arrayHeader = new byte[headerArrayLen + 8];
        byte[] body = new byte[messageBodyLen];

        inData = ByteBuffer.wrap( h.getMessages().get( 0 ) );
        inData.get( header1 );
        // Note: Message.java is assuming that the array length has padding
        // (or something like that) for this message type, so we need to first
        // copy the first 4 bytes, and then the remainder after that.
        inData.get(arrayHeader, 0, 4);
        inData.get(arrayHeader, 8, arrayHeader.length - 8);
        inData.get(body);

        Message m;
        try {
            m = MessageFactory.createMessage(type, header1, arrayHeader, body, h.getFileDescriptors() );
        } catch (DBusException dbe) {
            logger.debug("", dbe);
            throw dbe;
        } catch (RuntimeException exRe) { // this really smells badly!
            logger.debug("", exRe);
            throw exRe;
        }
        logger.debug("=> {}", m);

        return m;
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

    private native MsgHdr readNative( int handle ) throws IOException;

}
