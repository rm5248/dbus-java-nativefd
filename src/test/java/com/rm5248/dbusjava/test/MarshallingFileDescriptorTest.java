package com.rm5248.dbusjava.test;

import static org.junit.jupiter.api.Assertions.*;

import java.io.FileNotFoundException;
import java.io.IOException;
import jnr.constants.platform.AddressFamily;
import jnr.constants.platform.Sock;
import jnr.posix.POSIXFactory;
import org.freedesktop.dbus.FileDescriptor;

import org.freedesktop.dbus.annotations.DBusInterfaceName;
import org.freedesktop.dbus.connections.impl.DBusConnection;
import org.freedesktop.dbus.connections.impl.DBusConnection.DBusBusType;
import org.freedesktop.dbus.exceptions.DBusException;
import org.freedesktop.dbus.exceptions.DBusExecutionException;
import org.freedesktop.dbus.interfaces.DBusInterface;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class MarshallingFileDescriptorTest {

    private static jnr.posix.POSIX POSIX = POSIXFactory.getPOSIX();

    private static final String TEST_OBJECT_PATH = "/TestFileDescriptor";
    private static final String TEST_BUSNAME = "foo.bar.TestFileDescriptor";
    
    private DBusConnection serverConn;
    private DBusConnection clientConn;

    private int[] filedescriptors = {0, 0};
    private byte[] tosend = "this is a test".getBytes();
    
    @BeforeEach
    public void before() throws DBusException, FileNotFoundException, IOException {
        serverConn = DBusConnection.getConnection(DBusBusType.SESSION);
        clientConn = DBusConnection.getConnection(DBusBusType.SESSION);
        serverConn.setWeakReferences(true);
        clientConn.setWeakReferences(true);
        serverConn.requestBusName(TEST_BUSNAME);

        int ret = POSIX.socketpair(AddressFamily.AF_UNIX.intValue(), Sock.SOCK_STREAM.intValue(), 0, filedescriptors);

        System.out.println( "Filedescriptors[0]=" + filedescriptors[0] + " Filedescriptors[1]=" + filedescriptors[1] );

        assertTrue(ret >= 0);
        assertTrue(filedescriptors[0] > 0);
        assertTrue(filedescriptors[1] > 0);

        GetFileDescriptor gfd = new GetFileDescriptor( new FileDescriptor( filedescriptors[1] ) );
        
        serverConn.exportObject(TEST_OBJECT_PATH, gfd);
    }

    @AfterEach
    public void after() throws IOException {
        DBusExecutionException dbee = serverConn.getError();
        if (null != dbee) {
            throw dbee;
        }
        dbee = clientConn.getError();
        if (null != dbee) {
            throw dbee;
        }
        
        clientConn.disconnect();
        serverConn.disconnect();
    }
    
    @Test
    public void testFileDescriptor() throws DBusException, IOException {
        DBusInterface remoteObject = clientConn.getRemoteObject("foo.bar.TestFileDescriptor", TEST_OBJECT_PATH, IFileDescriptor.class);

        assertTrue(remoteObject instanceof IFileDescriptor, "Expected instance of GetFileDescriptor");
        
        FileDescriptor fileDescriptor = ((IFileDescriptor) remoteObject).getFileDescriptor();
        assertNotNull(fileDescriptor, "Descriptor should not be null");
        
        int receivedFdId = fileDescriptor.getIntFileDescriptor();

        assertNotEquals( receivedFdId, filedescriptors[ 1 ] );

        if( POSIX.write( filedescriptors[ 0 ], tosend, tosend.length ) < 0 ){
            System.out.println( "Can't write?" );
        }

        System.out.println( "Received int FD value " + receivedFdId );
        byte[] data = new byte[ tosend.length ];
        int len = POSIX.read( receivedFdId, data, data.length );
        if( len < 0 ){
            fail( "Can't read" );
        }

        String originalString = new String( tosend, 0, tosend.length );
        String receivedString = new String( data, 0, len );
        assertEquals( originalString, receivedString );
    }
    
    // ==================================================================================================

    public static class GetFileDescriptor implements IFileDescriptor {

        private final FileDescriptor fileDescriptor;

        public GetFileDescriptor(FileDescriptor _descriptor) {
            fileDescriptor = _descriptor;
        }
        
        @Override
        public boolean isRemote() {
            return false;
        }

        @Override
        public String getObjectPath() {
            return null;
        }

        @Override
        public FileDescriptor getFileDescriptor() {
            return fileDescriptor;
        }
        
    }
    
    @DBusInterfaceName("foo.bar.TestFileDescriptor")
    public static interface IFileDescriptor extends DBusInterface {

        FileDescriptor getFileDescriptor();
    }
}
