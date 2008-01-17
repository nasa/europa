import sys
import getopt
import UBO

#example :
#export TEST_DIR=$PLASMA_HOME/../benchmarks/UBO/data  
#python runTests.py -t10 $TEST_DIR benchmarks_ubo10.txt benchmarks_ubo20.txt
# options -t timeout in seconds
timeoutSecs = 20 

opts, args = getopt.getopt(sys.argv[1:], 't:')
for o, a in opts:
        if o == "-t":            
            timeoutSecs = int(a)
            
#print 'timeout is:'+str(timeoutSecs)             
test_dir = args[0]
for file in args[1:]:
    tr = UBO.TestRunner(test_dir,file)
    tr.runTests(timeoutSecs)

