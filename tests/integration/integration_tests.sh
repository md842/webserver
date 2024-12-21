#! /bin/bash

# Usage:
# $1: Path to file containing expected output
# $2: Command to run (curl, nc)
# $3: Options for the command specified in $2
# $4: Path to file containing netcat input (omit for curl)

rm results.txt # Remove previous test results
touch results.txt # Create new test results file

integration_test (){
  NUM=$(($NUM+1)) # Increment test number counter
  if [ $2 = "curl" ] # If command is curl
  then
    $2 $3 # Run curl, background process not necessary as curl terminates on its own
  else
    $2 $3 < $4 > output & # Run netcat command as background process, redirecting stdin to $4, stdout to output
  fi

  sleep 0.1 # Small delay for server processing/output to become available
  diff output $1 # Compare actual output to expected output
  DIFF_CODE=$? # diff uses exit codes to indicate file equality

  if [ $DIFF_CODE != 0 ] # diff returned nonzero; files are not equal
  then
    if [ $2 = "curl" ] # If command is curl
      then
        printf "Test $NUM FAIL: $2 $3\n" >> results.txt # Print info about curl
      else
        printf "Test $NUM FAIL: $2 $3 < $4\n" >> results.txt # Print info about netcat
    fi
    kill $WEBSERVER_PID # Shut down web server on test fail
    exit $DIFF_CODE # Exit early on test fail
  fi

  if [ $2 = "curl" ] # If command is curl
  then
    printf "Test $NUM PASS: $2 $3\n" >> results.txt # Print info about curl
  else
    printf "Test $NUM PASS: $2 $3 < $4\n" >> results.txt # Print info about netcat
  fi
}

NUM=0 # Initialize test number counter
../../build/bin/server configs/local_config.conf & # Ending with & runs command as background process
WEBSERVER_PID=$! # Save PID of web server to shut it down after tests are done
sleep 0.1 # Give the server time to start up

# Function call  $1: Expected output file                             $2: Command   $3: Options                                                   $4 Netcat input file (omit for curl)
integration_test "../../frontend/build/index.html"                    "curl"        "-o output -s http://localhost:8080/"
integration_test "../../frontend/public/test.txt"                     "curl"        "-o output -s http://localhost:8080/test.txt"
integration_test "../nc/outputs/leave_dir.txt"                        "nc"          "localhost 8080"                                              "../nc/inputs/leave_dir.txt"
integration_test "../nc/outputs/invalid_method.txt"                   "nc"          "localhost 8080"                                              "../nc/inputs/invalid_method.txt"
integration_test "../nc/outputs/content_length_non_numeric_get.txt"   "nc"          "localhost 8080"                                              "../nc/inputs/content_length_non_numeric_get.txt"
integration_test "../nc/outputs/content_length_non_numeric_post.txt"  "nc"          "localhost 8080"                                              "../nc/inputs/content_length_non_numeric_post.txt"
integration_test "../nc/outputs/excessive_payload_get.txt"            "nc"          "localhost 8080"                                              "../nc/inputs/excessive_payload_get.txt"
integration_test "../nc/outputs/excessive_payload_post.txt"           "nc"          "localhost 8080"                                              "../nc/inputs/excessive_payload_post.txt"
# Function call  $1: Expected output file                             $2: Command   $3: Options                                                   $4 Netcat input file (omit for curl)

kill $WEBSERVER_PID # Shut down web server after all tests have finished. Also ends any netcat background processes that are still alive.