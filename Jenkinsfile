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

      stage('setup-native-build-depends'){
          agent{ label 'debian10-openstack' }
          steps{
              sh '''
                  sudo apt-get -y install cmake build-essential
                 '''

              checkout scm

              sh '''
                  mvn compiler:compile@generate-jni-headers
                 '''
          }
      }
      
      stage('build-amd64'){
          agent{ label 'debian10-openstack' }
          steps{
              sh '''
                  mkdir -p target/amd64
                  cd target/amd64
                  cmake ../../src/main/jni
                  make
                  mkdir -p ../resources/amd64
                  cp *.so ../resources/amd64
                 '''
          }
      }

      stage('build-x86'){
          agent{ label 'debian10-openstack' }
          steps{
              sh '''
                  apt-get -y install gcc-multilib
                  mkdir -p target/i386
                  cd target/i386
                  CC="gcc -m32" cmake ../../src/main/jni
                  make
                  mkdir -p ../resources/i386
                  cp *.so ../resources/i386
                 '''
          }
      }

      stage('build-arm'){
          agent{ label 'debian10-openstack' }
          steps{
              sh '''
                  apt-get -y install gcc-arm-linux-gnueabihf
                  mkdir -p target/armhf
                  cd target/armhf
                  CC=arm-linux-gnueabihf-gcc cmake ../../src/main/jni
                  make
                  mkdir -p ../resources/arm
                  cp *.so ../resources/arm
                 '''
          }
      }

      stage('Stash Binaries'){
          agent{ label 'debian10-openstack' }
          steps{
             stash includes: 'src/main/resources/**/*.so', name: 'libs'
          }
      }

      
      stage('Package'){
          steps{
              unstash 'libs'
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
