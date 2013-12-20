#! /bin/bash

export OMP_NUM_THREADS=2

# TEST1 = first file  - must exist
# TEST2 = second file - must exist
# OUT_FILE = resulted patch - can have any name

make runtest TEST1=$1 TEST2=$2 OUT_FILE=$3
