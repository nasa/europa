import sys
import UBO

#example :
#export TEST_DIR=$PLASMA_HOME/becnhmarks/UBO/data  
#python runTests.py $TEST_DIR benchmarks_ubo10.txt benchmarks_ubo20.txt
timeoutSecs = 60 # TODO: take this as an arg?
test_dir = sys.argv[1]
for file in sys.argv[2:]:
    tr = UBO.TestRunner(test_dir,file)
    tr.runTests(timeoutSecs)

