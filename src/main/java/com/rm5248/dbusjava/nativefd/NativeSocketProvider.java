package com.rm5248.dbusjava.nativefd;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import java.nio.channels.SocketChannel;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.logging.Level;
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

    private static final Logger logger = LoggerFactory.getLogger( NativeSocketProvider.class.getName() );

    static{
        loadNativeLibrary();
//        System.out.println( "==================================================" );
//        System.load( "/home/robert/NetBeansProjects/dbus-java-nativefd/target/linux-x86_64/libdbus-java-jni-connector.so" );
    }

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

    /**
     * Load the native library.
     *
     * There are two important system properties that can be set here:
     *
     * com.rm5248.dbusnative.lib.path - give the directory name that the JNI
     * code is located in
     *
     * com.rm5248.dbusnative.lib.name - explicitly give the
     * name of the library(the default is 'dbus-java-jni-connector')
     *
     * This is based largely off of SQLite-JDBC
     * ( https://github.com/xerial/sqlite-jdbc )
     */
    private static void loadNativeLibrary(){
        String nativeLibraryPath = System.getProperty( "com.rm5248.dbusnative.lib.path" );
        String nativeLibraryName = System.getProperty( "com.rm5248.dbusnative.lib.name" );

        if( nativeLibraryName == null ){
            nativeLibraryName = System.mapLibraryName( "dbus-java-jni-connector" );
            if( nativeLibraryName.endsWith( "dylib" ) ){
                //mac uses jnilib instead of dylib for some reason
                nativeLibraryName = nativeLibraryName.replace( "dylib", "jnilib" );
            }
            logger.debug( "No native library name provided, using default of {}",
                    nativeLibraryName);
        }

        if( nativeLibraryPath != null ){
            logger.debug( "Native library path of {} provided", nativeLibraryPath );
            File libToLoad = new File( nativeLibraryPath, nativeLibraryName );
            logger.debug( "Loading library {}", libToLoad.getAbsolutePath() );
            System.load( libToLoad.getAbsolutePath() );
            return;
        }

        //if we get here, that means that we must extract the JNI from the jar
        try{
            File extractedLib;
            Path tempFolder;
            String osName;
            String arch;
            InputStream library;

            osName = System.getProperty( "os.name" );
            if( osName.contains( "Windows" ) ){
                osName = "Windows";
            } else if( osName.contains( "Mac" ) || osName.contains( "Darwin" ) ){
                osName = "Mac";
            } else if( osName.contains( "Linux" ) ){
                osName = "Linux";
            } else{
                osName = osName.replaceAll( "\\W", "" );
            }

            arch = System.getProperty( "os.arch" );
            arch.replaceAll( "\\W", "" );

            if( arch.equals( "x86_64" ) ){
                //map x86_64 to amd64 to stay consistent(needed for mac)
                arch = "amd64";
            }

            //create the temp folder to extract the library to
            tempFolder = Files.createTempDirectory( "dbus-java" );
            tempFolder.toFile().deleteOnExit();

            logger.debug( "Created temp folder of {}", tempFolder );

            extractedLib = new File( tempFolder.toFile(), nativeLibraryName );

            String fileToExtract = "/" + osName + "/" + arch + "/" + nativeLibraryName;
            logger.debug( "About to extract {} from JAR", fileToExtract );
            //now let's extract the proper library
            library = NativeSocketProvider.class.getResourceAsStream( fileToExtract );

            if( library == null ){
                throw new UnsatisfiedLinkError( "Unable to extract native library from JAR:"
                        + fileToExtract + "."
                );
            }

            Files.copy( library, extractedLib.toPath() );
            extractedLib.deleteOnExit();

            System.load( extractedLib.getAbsolutePath() );
        } catch( IOException e ){
            throw new UnsatisfiedLinkError( "Unable to create temp directory or extract: " + e.getMessage() );
        }
    }

}
