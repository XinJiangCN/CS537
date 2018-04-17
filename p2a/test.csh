#! /bin/csh -f
set TEST_HOME = /afs/cs.wisc.edu/p/course/cs537-remzi/tests
set source_file = wish.c
set binary_file = wish
set bin_dir = /afs/cs.wisc.edu/p/course/cs537-remzi/tests/bin
set test_dir = /afs/cs.wisc.edu/p/course/cs537-remzi/tests/tests-wish
${bin_dir}/generic-tester.py $argv[*] -s $source_file -b $binary_file -t $test_dir
