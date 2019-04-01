#!/bin/bash

generate_runner_flags_help () {
    echo "-d            run debugger"
    echo "-f ftype      what floating point type to use (FLOAT or DOUBLE)"
    echo "-c            use CPU only backend"
}

# Parse flags
GENERATOR_MAKEFILE="MakefileCUDA"
FTYPE="FLOAT"
while getopts ":df:ch" option; do
    case $option in
        d) BUILD_MODEL_ARGS="$BUILD_MODEL_ARGS -d";DEBUG=1;;
        f) FTYPE="$OPTARG";;
        c) BUILD_MODEL_ARGS="$BUILD_MODEL_ARGS -c";;
        h) generate_runner_help; exit;;
        ?) generate_runner_help; exit;;
        :) generate_runner_help; exit;;
    esac
done

# Shift off arguments already processed
shift $((OPTIND -1))