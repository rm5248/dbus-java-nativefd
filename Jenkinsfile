pipeline {
   agent any

   tools {
      maven "maven 3.6.0"
      jdk "jdk-11"
   }

   stages {
       stage('Cleanup'){
           steps{
               cleanWs()
           }
       }
       stage('Checkout'){
        steps {
            checkout scm
         }
       }

      stage('Launch Native Code VM'){
          steps{
              openstackMachine cloud: 'dreamhost', template: 'debian10-builder'
          }
      }

      stage('Build') {
        steps{
            sh 'mvn compile'
        }
      }
      
      stage('Test'){
          steps{
            
            sh '''
# launch up an instance of the dbus and source the data output
# to see what the address is and what the PID is
TMPFILE=$(mktemp)
dbus-launch > $TMPFILE

. $TMPFILE
export DBUS_SESSION_BUS_ADDRESS

mvn test
# get the exit code 
EXIT_CODE=$?

kill $DBUS_SESSION_BUS_PID
rm $TMPFILE

exit $EXIT_CODE
            '''
        }
        
      }
      
      stage('build-amd64'){
          agent{ label 'debian10-openstack' }
          steps{
              sh '''
                  sudo apt-get -y install cmake build-essential git
                 '''

              checkout scm

              sh '''
                  cd src/main/jni
                  cmake .
                  make
                 '''

             stash includes: 'src/main/jni/*.so', name: 'lib-amd64'
          }
      }

/*
      stage('build-x86'){
          steps{
              sh '''
mkdir -p target/x86-cmake
cd target/x86-cmake
CC="gcc -m32" ../dependency/cmake/bin/cmake ../../src/main/jni
make
'''
          }
      }

      stage('build-arm'){
          steps{
              sh '''
mkdir -p target/arm-cmake
cd target/arm-cmake
CC=arm-linux-gnueabihf-gcc ../dependency/cmake/bin/cmake ../../src/main/jni
make
'''
          }
      }

      stage('copy-binaries-to-resources'){
          steps{
              sh '''
mkdir -p src/main/resources/amd64
mkdir -p src/main/resources/i386
mkdir -p src/main/resources/arm

cp target/amd64-cmake/*.so src/main/resources/amd64
cp target/x86-cmake/*.so src/main/resources/i386
cp target/arm-cmake/*.so src/main/resources/arm
'''
          }
      }
*/
      
      stage('Package'){
          steps{
              sh 'mvn -P add-precompiled-binaries -Dmaven.test.skip=true package'
          }

         post {
            // If Maven was able to run the tests, even if some of the test
            // failed, record the test results and archive the jar file.
            success {
               junit 'target/surefire-reports/TEST-*.xml'
               archiveArtifacts 'target/*.jar'
            }
         }
      }
   }
}
