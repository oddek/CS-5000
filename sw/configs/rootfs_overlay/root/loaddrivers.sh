#!/bin/sh

./unload_cma
./unload_generator
./unload_verifier

./load_cma
./load_generator
./load_verifier