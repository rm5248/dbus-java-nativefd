pipeline {
   agent { label 'master' }

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

      stage('Build Native Code'){
          agent{ label 'debian10-openstack' }
          stages{ 
              stage('Install Tools'){
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
                  steps{
                      sh '''
                          mkdir -p target/amd64
                          cd target/amd64
                          cmake ../../src/main/jni
                          make
                         '''
                  }
              }
        
              stage('build-x86'){
                  steps{
                      sh '''
                          sudo apt-get -y install gcc-multilib
                          mkdir -p target/i386
                          cd target/i386
                          CC="gcc -m32" cmake ../../src/main/jni
                          make
                         '''
                  }
              }

              stage('build-armhf'){
                  steps{
                      sh '''
                          sudo apt-get -y install gcc-arm-linux-gnueabihf
                          mkdir -p target/armhf
                          cd target/armhf
                          CC=arm-linux-gnueabihf-gcc cmake ../../src/main/jni
                          make
                         '''
                  }
              }
        
              stage('Stash Binaries'){
                  steps{
                     sh '''
                            mkdir -p src/main/resources/amd64
                            mkdir -p src/main/resources/i386
                            mkdir -p src/main/resources/armhf

                            cp target/amd64/*.so src/main/resources/amd64
                            cp target/i386/*.so src/main/resources/i386
                            cp target/armhf/*.so src/main/resources/armhf
                        '''
                     stash includes: 'src/main/resources/**/*.so', name: 'libs'
                  }
              }

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
