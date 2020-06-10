# dbus-java-nativefd

[![Build Status](https://jenkins.rm5248.com/buildStatus/icon?job=DBus-Java+Projects%2Fmultibranch-test%2Fmaster)](https://jenkins.rm5248.com/job/DBus-Java%20Projects/job/multibranch-test/job/master/)

This repo contains an implementation of `IServiceProvider` for use
with [dbus-java](https://github.com/hypfvieh/dbus-java) that allows
for sending and receiving `FileDescriptor`s

To use: simply add to your project via Maven.

Example:

```
<dependency>
    <groupId>com.rm5248</groupId>
    <artifactId>dbus-java-nativefd</artifactId>
    <version>1.0</version>
</dependency>
```

Note that dbus-java needs to be version 3.2.1 or greater.  This is explicitly
called out in the pom.xml

# Native Code

This project uses a native library in order to send the file descriptors
between processors.  This code is packaged in the JAR file and automatically
extracted at runtime.

If you need a custom build for some reason, you may use the CMake project in
`src/main/jni`.  Once built, you may tell dbus-java-nativefd to load it using
the following Java system properties(set with the -D argument to java):

```
com.rm5248.dbusnative.lib.path - give the directory name that the JNI code is 
located in

com.rm5248.dbusnative.lib.name - explicitly give the name of the library(the 
default is 'dbus-java-jni-connector')
```

Binaries are provided for Linux(amd64, i386, armhf).

# License

Apache 2.0
