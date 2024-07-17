#!/bin/bash

if ! command -v gcc &> /dev/null; then
        echo "install gcc. Verify with gcc --version."
        exit 1
fi

# vars to make renaming easy
broker="smbbroker"
pub="smbpublish"
sub="smbsubscribe"

if ! [ -d "bin" ]; then
        # create bin dir if needed
        mkdir bin;
fi

# compile the three programs
# attend the -test prefix
# in case compilation fails

gcc src/$broker.c -o bin/$broker-test
if [ $? -eq 0 ]; then
        echo "$broker compiled!"
        mv bin/$broker-test bin/$broker
fi

gcc src/$pub.c -o bin/$pub-test
if [ $? -eq 0 ]; then
        echo "$pub compiled!"
        mv bin/$pub-test bin/$pub
fi

gcc src/$sub.c -o bin/$sub-test
if [ $? -eq 0 ]; then
        echo "$sub compiled!"
        mv bin/$sub-test bin/$sub
fi
