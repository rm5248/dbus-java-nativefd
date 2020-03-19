package com.rm5248.dbusjava.nativefd;

import java.util.ArrayList;
import java.util.List;
import org.freedesktop.dbus.FileDescriptor;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *  msg_name=null,
  msg_namelen=0,
  msg_iov=[
    iovec {
      iov_base=jnr.ffi.provider.jffi.DirectMemoryIO[address=0x7fee7c8736c0],
      iov_len=84,
    }
  ],
  msg_control=[
    cmsg {
      cmsg_len=28
      cmsg_level=1
      cmsg_type=2
      cmsg_data=java.nio.HeapByteBuffer[pos=0 lim=12 cap=12]
    }
  ],
  msg_controllen=32
  msg_iovlen=1,
  msg_flags=0,
}
 */
public class MsgHdr {

    private List<byte[]> m_messages;
    private List<FileDescriptor> m_fileDescriptors;

    public MsgHdr(){
        m_messages = new ArrayList<byte[]>();
        m_fileDescriptors = new ArrayList<FileDescriptor>();
    }

    public MsgHdr( byte[] data, int[] fileDescriptors ){
        m_messages = new ArrayList<byte[]>();
        m_messages.add( data );

        m_fileDescriptors = new ArrayList<FileDescriptor>( fileDescriptors == null ? 1 : fileDescriptors.length );
        if( fileDescriptors != null ){
            for( int x = 0; x < fileDescriptors.length; x++ ){
                m_fileDescriptors.add( new FileDescriptor( fileDescriptors[ x ] ) );
            }
        }
    }

    public void addMessageToSend( byte[] msg ){
        m_messages.add( msg );
    }

    public void addFiledescriptorToSend( FileDescriptor fd ){
        m_fileDescriptors.add( fd );
    }

    public List<byte[]> getMessages(){
        return m_messages;
    }

    public List<FileDescriptor> getFileDescriptors(){
        return m_fileDescriptors;
    }

    public String toString(){
        StringBuilder builder = new StringBuilder();

        builder.append( "MsgHdr: " ).append( System.lineSeparator() )
                .append( "  " ).append( "msg_iov=[" ).append( System.lineSeparator() );

        for( byte[] message : m_messages ){
            builder.append( "    " ).append( "len: " ).append( message.length );
            builder.append( System.lineSeparator() );
        }

        builder.append( "]," ).append( System.lineSeparator() );

        builder.append( "  " ).append( "msg_control=[" ).append( System.lineSeparator() );

        for( FileDescriptor fd : m_fileDescriptors ){
            builder.append( "    " ).append( fd );
            builder.append( System.lineSeparator() );
        }

        builder.append( "]," ).append( System.lineSeparator() );

        return builder.toString();
    }
}
