#!/bin/sh

FOLDER="./build"

# check if one argument is provided
if [ $# == 1 ]; then
    # printf "Usage: $0 target build location"
    # The user costumized target build directory
    FOLDER=$1
fi

# The directory of the current script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# build
mkdir $FOLDER
cd $FOLDER
cmake $SCRIPT_DIR
make