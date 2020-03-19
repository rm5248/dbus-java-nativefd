package com.rm5248.dbusjava.nativefd;

import java.io.IOException;
import java.net.Socket;
import java.nio.channels.SocketChannel;
import jnr.unixsocket.UnixSocketChannel;
import org.freedesktop.dbus.spi.IMessageReader;
import org.freedesktop.dbus.spi.IMessageWriter;
import org.freedesktop.dbus.spi.ISocketProvider;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 */
public class NativeSocketProvider implements ISocketProvider {

    static{
        System.out.println( "==================================================" );
        System.load( "/home/robert/NetBeansProjects/dbus-java-nativefd/target/linux-x86_64/libdbus-java-jni-connector.so" );
    }

    private final Logger logger = LoggerFactory.getLogger(getClass());

    private boolean m_hasFiledescriptorSupport;
    private NativeMessageReader m_nativeMessageReader;
    private NativeMessageWriter m_nativeMessageWriter;

    public NativeSocketProvider(){
        logger.debug( "new NativeSocketProvider" );
        m_hasFiledescriptorSupport = false;
    }

    @Override
    public IMessageReader createReader(Socket _socket) throws IOException {
        if( !m_hasFiledescriptorSupport ){
            return null;
        }

        SocketChannel sc = _socket.getChannel();
        
        if( sc instanceof UnixSocketChannel ){
            int fd = ((UnixSocketChannel) sc).getFD();
            m_nativeMessageReader = new NativeMessageReader( fd );
            return m_nativeMessageReader;
        }

        return null;
    }

    @Override
    public IMessageWriter createWriter(Socket _socket) throws IOException {
        if( !m_hasFiledescriptorSupport ){
            return null;
        }

        SocketChannel sc = _socket.getChannel();

        if( sc instanceof UnixSocketChannel ){
            int fd = ((UnixSocketChannel) sc).getFD();
            m_nativeMessageWriter = new NativeMessageWriter( fd );
            return m_nativeMessageWriter;
        }

        return null;
    }

    @Override
    public void setFileDescriptorSupport(boolean _support) {
        m_hasFiledescriptorSupport = _support;
    }

    @Override
    public boolean isFileDescriptorPassingSupported() {
        return true;
    }

}
