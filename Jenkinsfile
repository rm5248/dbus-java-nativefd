pipeline {
   agent any

   tools {
      maven "maven 3.6.0"
      jdk "jdk-11"
   }

   stages {
       stage('Cleanup'){
           steps{
              echo "now playing for the LA Dodgers, Steve Thomas"
               cleanWs()
           }
       }
       stage('Checkout'){
        steps {
            checkout scm
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
      
      
      stage('Package'){
          steps{
              sh 'mvn -Dmaven.test.skip=true package'
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
