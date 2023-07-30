#!/bin/bash

rm -rf tmpFiles && mkdir tmpFiles
rm -rf testDir && mkdir testDir
cd testDir
touch file1 file2 file3
mkdir subdir1 subdir2 subdir3

compare_actual_expected () {
	diff ../tmpFiles/actual.txt ../tmpFiles/expected.txt
	if [ $? == 0 ]
	then
		echo "SUCCESS"
	else
		echo "FAILURE"
	fi
}

################
# 'ls'         #
################
echo "Testing 'ls'"

# -C to force multi-column, which would be the default in terminal.
../ls -C > ../tmpFiles/actual.txt
echo "file1       file2       file3       subdir1     subdir2     subdir3     " > ../tmpFiles/expected.txt
compare_actual_expected

################
# 'ls -l'      #
################
echo "Testing 'ls -l'"
touch -t 202307192200 *
../ls -l > ../tmpFiles/actual.txt
echo "total 0" > ../tmpFiles/expected.txt
echo "-rw-r--r--  1 $(id -un)  $(id -gn)   0 Jul 19 22:00 file1"   >> ../tmpFiles/expected.txt
echo "-rw-r--r--  1 $(id -un)  $(id -gn)   0 Jul 19 22:00 file2"   >> ../tmpFiles/expected.txt
echo "-rw-r--r--  1 $(id -un)  $(id -gn)   0 Jul 19 22:00 file3"   >> ../tmpFiles/expected.txt
echo "drwxr-xr-x  2 $(id -un)  $(id -gn)  64 Jul 19 22:00 subdir1" >> ../tmpFiles/expected.txt
echo "drwxr-xr-x  2 $(id -un)  $(id -gn)  64 Jul 19 22:00 subdir2" >> ../tmpFiles/expected.txt
echo "drwxr-xr-x  2 $(id -un)  $(id -gn)  64 Jul 19 22:00 subdir3" >> ../tmpFiles/expected.txt
compare_actual_expected
