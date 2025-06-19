#! /bin/bash

# Set file paths for test output and result files (relative to project root)
OUTPUT=tests/integration/last_test_output.txt
RESULT=tests/integration/last_test_result.txt

# Usage:
# $1: Path to file containing expected output
# $2: Command to run (curl, nc)
# $3: Options for the command specified in $2
# $4: Path to file containing netcat input (omit for curl)

integration_test (){
  NUM=$(($NUM+1)) # Increment test number counter
  if [ $2 = "curl" ] # If command is curl
  then
    $2 $3 # Run curl, background process not necessary as curl terminates on its own
  else
    $2 $3 < $4 > $OUTPUT & # Run netcat command as background process, redirecting stdin to $4, stdout to output
  fi

  sleep 0.1 # Small delay for server processing/output to become available
  diff $OUTPUT $1 # Compare actual output to expected output
  DIFF_CODE=$? # diff uses exit codes to indicate file equality

  if [ $DIFF_CODE != 0 ] # diff returned nonzero; files are not equal
  then
    if [ $2 = "curl" ] # If command is curl
      then
        printf "Test $NUM FAIL: $2 $3\n" >> $RESULT # Print info about curl
      else
        printf "Test $NUM FAIL: $2 $3 < $4\n" >> $RESULT # Print info about netcat
    fi
    kill $WEBSERVER_PID # Shut down web server on test fail
    exit $DIFF_CODE # Exit early on test fail
  fi

  if [ $2 = "curl" ] # If command is curl
  then
    printf "Test $NUM PASS: $2 $3\n" >> $RESULT # Print info about curl
  else
    printf "Test $NUM PASS: $2 $3 < $4\n" >> $RESULT # Print info about netcat
  fi
}

cd ../../ # Move to project root directory
rm $RESULT # Remove previous test results
touch $RESULT # Create new test results file
NUM=0 # Initialize test number counter
./build/bin/server configs/local_config.conf & # Ending with & runs command as background process
WEBSERVER_PID=$! # Save PID of web server to shut it down after tests are done
sleep 0.1 # Give the server time to start up

# Function call  $1: Expected output file                       $2: Command   $3: Options                                                   $4 Netcat input file (omit for curl)
integration_test "frontend/build/index.html"                    "curl"        "-k -o $OUTPUT -s https://localhost:8080/"
integration_test "frontend/public/test.txt"                     "curl"        "-k -o $OUTPUT -s https://localhost:8080/test.txt"
integration_test "tests/nc/outputs/leave_dir.txt"               "nc"          "localhost 8081"                                              "tests/nc/inputs/leave_dir.txt"
integration_test "tests/nc/outputs/invalid_method.txt"          "nc"          "localhost 8081"                                              "tests/nc/inputs/invalid_method.txt"
integration_test "tests/nc/outputs/redirect_test.txt"           "nc"          "localhost 8079"                                              "tests/nc/inputs/redirect_test.txt"
# Function call  $1: Expected output file                       $2: Command   $3: Options                                                   $4 Netcat input file (omit for curl)

kill $WEBSERVER_PID # Shut down web server after all tests have finished. Also ends any netcat background processes that are still alive.